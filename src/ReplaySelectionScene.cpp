#include "ReplaySelectionScene.h"
#include "natural_sort.h"
#include "ReplayIO.h"
#include "SelectionScene.h"

#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <cassert>


namespace hg {
    variant<move_function<std::vector<InputList>()>, SceneAborted_tag> run_replay_selection_scene(
        hg::RenderWindow &window, std::string const &levelName)
    {
        boost::filesystem::path win_replay = boost::filesystem::path("levels") / levelName / "win.replay";
        //Get levels/levelName.lvl/win.replay
        
        //Add list of files: replays/levelName/*.replay (Sorted)
        bool includesWinReplay{false};
        std::vector<boost::filesystem::path> optionList;
        if (exists(win_replay))
        {
            optionList.push_back(win_replay);
            includesWinReplay = true;
        }
        {
            boost::filesystem::path replaysPath = boost::filesystem::path("replays/") / boost::filesystem::path(levelName).stem();
            if (exists(replaysPath) && is_directory(replaysPath)) {
                boost::push_back(
                    optionList,
                    boost::make_iterator_range(boost::filesystem::directory_iterator(replaysPath), boost::filesystem::directory_iterator())
                    | boost::adaptors::filtered([](auto const &entry) { return entry.path().extension() == ".replay"; })
                    | boost::adaptors::transformed([](auto const &entry) { return entry.path(); }));

                std::sort(
                    boost::begin(optionList) + (includesWinReplay ? 1 : 0),
                    boost::end(optionList),
                    [](auto const &l, auto const &r)
                    {
                        return natural_less(l.stem().string(), r.stem().string());
                    });
            }
        }
        std::vector<std::string> optionStrings;
        boost::push_back(optionStrings, optionList | boost::adaptors::transformed([](auto const &path) {return path.stem().string();}));
        variant<std::size_t, SceneAborted_tag> selectedOption = run_selection_scene(window, optionStrings);

        if (selectedOption.active<SceneAborted_tag>())
        {
            return SceneAborted_tag{};
        }
        else
        {
            assert(selectedOption.active<SceneAborted_tag>(std::size_t));
        }
        boost::filesystem::path selectedPath{ optionList[selectedOption.get<std::size_t>()] };
        //return load function for selected replay.
        return move_function<std::vector<InputList>()>{[selectedPath] { return loadReplay(selectedPath.string()); }};
    }
}
