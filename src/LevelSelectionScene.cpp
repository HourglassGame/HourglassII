#include "LevelSelectionScene.h"
#include <boost/range/iterator_range.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm/adjacent_find.hpp>
#include <boost/filesystem.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Sleep.hpp>
#include "Maths.h"
#include "natural_sort.h"
#include "LevelLoader.h"
#include "SelectionScene.h"
#include <algorithm>

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
    hg::RenderWindow &window,
    GLFWWindow &windowglfw,
    VulkanEngine& vulkanEng,
    VulkanRenderer& vkRenderer,
    std::string &levelName)
{
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
    boost::push_back(optionStrings, levelPaths | boost::adaptors::transformed([](auto const &path) {return path.stem().string();}));

    int levelIndex = 0;
    for (auto it = optionStrings.begin(); it != optionStrings.end(); ++it) {
        if (*it == levelName) {
            levelIndex = it - optionStrings.begin();
            break;
        }
    }

    std::variant<std::size_t, SceneAborted_tag> selectedOption = run_selection_scene(
        window, windowglfw, levelIndex, optionStrings, vulkanEng, vkRenderer);

    if (std::holds_alternative<SceneAborted_tag>(selectedOption))
    {
        return SceneAborted_tag{};
    }
    else
    {
        assert(std::holds_alternative<std::size_t>(selectedOption));
    }
    boost::filesystem::path selectedPath{ levelPaths[std::get<std::size_t>(selectedOption)] };
    {
        auto levelPathString = selectedPath.string();
        return LoadLevelFunction{
            selectedPath.filename().string(),
            move_function<TimeEngine(hg::OperationInterrupter &)>(
                CachingTimeEngineLoader(selectedPath)),
            move_function<LoadedLevel(TimeEngine &&)>(
                [levelPathString](TimeEngine &&timeEngine) -> LoadedLevel {
                    auto wall = std::make_unique<sf::Image>(loadAndBakeWallImage(timeEngine.getWall()));
                    auto positionColours = std::make_unique<sf::Image>(loadAndBakePositionColourImage(timeEngine.getWall()));
                    return {
                        std::move(timeEngine),
                        loadLevelResources(levelPathString, "GlitzData"),
                        std::move(wall),
                        std::move(positionColours)
                    };
                }
            )
        };
    }
}
}
