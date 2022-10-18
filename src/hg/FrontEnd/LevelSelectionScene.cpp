#include "LevelSelectionScene.h"
#include <boost/range/iterator_range.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm/adjacent_find.hpp>
#include <boost/filesystem.hpp>
#include "hg/Util/Maths.h"
#include "hg/Util/natural_sort.h"
#include "hg/TimeEngine/LevelLoader.h"
#include "SelectionScene.h"
#include <regex>

namespace hg {
struct CachingTimeEngineLoader
{
	CachingTimeEngineLoader(boost::filesystem::path levelPath) :
		levelPath(std::move(levelPath)),
		previouslyLoadedLevel(),
		previouslyLoadedTimeEngine()
	{
	}
	TimeEngine operator()(hg::OperationInterrupter &interrupter) const
	{
		Level newLevel = loadLevelFromFile(
			levelPath,
			interrupter);
		if (previouslyLoadedLevel && *previouslyLoadedLevel == newLevel && previouslyLoadedTimeEngine)
		{
			return *previouslyLoadedTimeEngine;
		}
		auto newTimeEngine = TimeEngine(
			Level(newLevel),
			interrupter);
		{
			auto timeEngineCopy(std::make_unique<TimeEngine>(newTimeEngine));//TODO: Add and use 'copy constructor' that accepts interrupter
			auto levelCopy(std::make_unique<Level>(newLevel));
			previouslyLoadedTimeEngine = std::move(timeEngineCopy);
			previouslyLoadedLevel = std::move(levelCopy);
		}
		return newTimeEngine;
	}
private:
	boost::filesystem::path levelPath;
	//TODO: Consider adding mutex to satisfy 'logical const implies threadsafe' "rule".
	std::unique_ptr<Level> mutable previouslyLoadedLevel;
	std::unique_ptr<TimeEngine> mutable previouslyLoadedTimeEngine;
};

std::variant<LoadLevelFunction, SceneAborted_tag> run_level_selection_scene(
	GLFWWindow &windowglfw,
	VulkanEngine &vulkanEng,
	VulkanRenderer &vkRenderer,
	std::string &levelName,
	int position,
	int page,
	bool unlockAll)
{
	std::vector<hg::PageState> levelMenuConf = std::vector<hg::PageState>();
	std::vector<hg::LevelState> pageLevels;
	
	// New stuff
	pageLevels = std::vector<hg::LevelState>();
	pageLevels.push_back(hg::LevelState("FlowUphill.lvl", "Flow Uphill", 0));
	pageLevels.push_back(hg::LevelState("GetDown.lvl", "Get Down", 0));
	pageLevels.push_back(hg::LevelState("GetDown_hard.lvl", "Get Down (bonus)", "GetDown.lvl"));
	pageLevels.push_back(hg::LevelState("Staircase.lvl", "Staircase", 0));
	pageLevels.push_back(hg::LevelState("Solitude.lvl", "Solitude", 0));
	pageLevels.push_back(hg::LevelState("Solitude_hard.lvl", "Solitude (bonus)", 0));
	levelMenuConf.push_back(hg::PageState("New Levels", 0, pageLevels));
	
	// Page 1
	pageLevels = std::vector<hg::LevelState>();
	pageLevels.push_back(hg::LevelState("1EasyStart.lvl", "1 - Easy Start", 0));
	pageLevels.push_back(hg::LevelState("2OpenAndClosed.lvl", "2 - Open and Closed", 1));
	pageLevels.push_back(hg::LevelState("3StandardBoxPuzzle.lvl", "3 - Standard Box Puzzle", 2));
	pageLevels.push_back(hg::LevelState("4NotSoStandard.lvl", "4 - Not So Standard", 2));
	pageLevels.push_back(hg::LevelState("5GoingUp.lvl", "5 - Going Up", "1EasyStart.lvl"));
	levelMenuConf.push_back(hg::PageState("Page 1", 0, pageLevels));

	// Page 2
	pageLevels = std::vector<hg::LevelState>();
	pageLevels.push_back(hg::LevelState("13FishInABarrel.lvl", "13 - Fish in a Barrel", 0));
	pageLevels.push_back(hg::LevelState("14WrongWay.lvl", "14 - Wrong Way", 0));
	pageLevels.push_back(hg::LevelState("15Reverse.lvl", "15 - Reverse", 1));
	levelMenuConf.push_back(hg::PageState("Page 2", 3, pageLevels));

	// Add all excess levels
	std::vector<hg::LevelState> unsortedLevels = std::vector<hg::LevelState>();
	
	std::vector<boost::filesystem::path> levelPaths;
	for (auto entry: boost::make_iterator_range(boost::filesystem::directory_iterator("levels/"), boost::filesystem::directory_iterator())) {
		if (is_directory(entry.status()) && entry.path().extension()==".lvl") {
			levelPaths.push_back(entry.path());
		}
	}
	boost::sort(
		levelPaths,
		[](boost::filesystem::path const& l,boost::filesystem::path const& r)
		{
			return natural_less(l.stem().string(), r.stem().string());
		});

	std::vector<std::string> optionStrings;
	std::regex e(".lvl");
	boost::push_back(optionStrings, levelPaths | boost::adaptors::transformed([](auto const &path) {return path.filename().string();}));
	int unsortedOnPage = 0;
	for (auto it = optionStrings.begin(); it != optionStrings.end(); ++it) {
		bool found = false;
		for (auto it_p = levelMenuConf.begin(); it_p != levelMenuConf.end(); ++it_p) {
			for (auto it_o = (*it_p).options.begin(); it_o != (*it_p).options.end(); ++it_o) {
				if ((*it) == (*it_o).name) {
					found = true;
					break;
				}
			}
			if (found) {
				break;
			}
		}
		if (!found) {
			std::string levelName = std::regex_replace((*it), e, "");
			unsortedLevels.push_back(hg::LevelState((*it), levelName, 0));
			unsortedOnPage += 1;
			if (unsortedOnPage >= 12) {
				levelMenuConf.push_back(hg::PageState("Unassigned Levels", 0, unsortedLevels));
				unsortedLevels = std::vector<hg::LevelState>();
				unsortedOnPage = 0;
			}
		}
	}
	if (unsortedOnPage > 0) {
		levelMenuConf.push_back(hg::PageState("Unassigned Levels", 0, unsortedLevels));
	}

	//std::cout << "page: " << std::to_string(page) << ", position: " << std::to_string(position) << ", perPage: " << std::to_string(perPage) << "\n" << std::flush;

	std::variant<LevelSelectionReturn, SceneAborted_tag> selectedOption = run_selection_page_scene(windowglfw, position, page, levelMenuConf, vulkanEng, vkRenderer, unlockAll);
	// Todo update position and page
	if (std::holds_alternative<SceneAborted_tag>(selectedOption))
	{
		return SceneAborted_tag{};
	}
	else
	{
		assert(std::holds_alternative<LevelSelectionReturn>(selectedOption));
	}
	
	boost::filesystem::path selectedPath{ boost::filesystem::path(std::string("levels/") + std::get<LevelSelectionReturn>(selectedOption).name)};
	{
		auto levelPathString = selectedPath.string();
		return LoadLevelFunction{
			selectedPath.filename().string(),
			std::get<LevelSelectionReturn>(selectedOption).position,
			std::get<LevelSelectionReturn>(selectedOption).page,
			move_function<TimeEngine(hg::OperationInterrupter &)>(
				CachingTimeEngineLoader(selectedPath)),
			move_function<LoadedLevel(TimeEngine &&)>(
				[levelPathString](TimeEngine &&timeEngine) -> LoadedLevel {
					return {
						std::move(timeEngine),
						loadLevelResources(levelPathString, "GlitzData")
					};
				}
			)
		};
	}
}
}
