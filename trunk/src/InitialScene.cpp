#include "InitialScene.h"
#include "RenderWindow.h"
#include "LoadingLevelScene.h"
#include "LoadedLevel.h"
#include "RunningGameScene.h"
#include "LuaError.h"
#include "move_function.h"
#include "MainMenuScene.h"
#include "ReplayIO.h"
#include "LevelSelectionScene.h"
#include <functional>
namespace hg {
struct RunGameResultVisitor {
    typedef variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>>result_type;
    template<typename Tag>
    variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>>  operator()(Tag t) const {
        return std::move(t);
    }
};
static variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>>
       loadAndRunLevel(hg::RenderWindow &window, hg::move_function<hg::LoadedLevel(hg::OperationInterrupter &)> const& levelLoadingFunction, hg::move_function<std::vector<InputList>()>&& replayLoadingFunction = {})
{
    hg::variant<hg::LoadedLevel, LoadingCanceled_tag>
              loading_outcome = load_level_scene(window, levelLoadingFunction);
    
    struct {
        hg::RenderWindow &window;
        hg::move_function<std::vector<InputList>()> &replayLoadingFunction;
        typedef variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>> result_type;
        variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>>
        operator()(LoadedLevel& level) const
        {
            if (replayLoadingFunction) {
                return run_game_scene(window, std::move(level), replayLoadingFunction()).visit(RunGameResultVisitor{});
            }
            else {
                return run_game_scene(window, std::move(level)).visit(RunGameResultVisitor{});
            }
           
        }
        variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>> operator()(LoadingCanceled_tag) const {
            return GameAborted_tag{};
        }
    } visitor = {window, replayLoadingFunction};
    return loading_outcome.visit(visitor);
}

int run_hourglassii() {
    //Create window
    hg::RenderWindow window(sf::VideoMode(640, 480), "Hourglass II");
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);
    //Do stuff!
    //Need a loop maybe, so that when a level is reset, it can come back to the loading screen.
    //... or something...?
    
    //The real problem here is that `run_level_scene` has highly complex outputs
    //Inputs:
    //Access to global realtime I/O (windowing, audio, (filesystem, other?))
    //
    //Outputs:
    //The next desired action, and the data required to perform that action
    // Quit
    // Restart Level
    // Load Replay (replay filepath)
    // Register Level Success (replay data)
    // Level Aborted
    // Error condition encountered (error details)
    // so on, so forth, need full list.
    
    try {
        while (true) {
            variant<RunALevel_tag, RunAReplay_tag, Exit_tag> main_menu_result = run_main_menu(window);
            std::vector<hg::InputList> replay;
            if (main_menu_result.active<Exit_tag>()) {
                return EXIT_SUCCESS;
            }
            else if (main_menu_result.active<RunAReplay_tag>()) {
                replay = loadReplay("replay");
            }
            else {
                assert(main_menu_result.active<RunALevel_tag>());
            }
            
            variant<hg::move_function<hg::LoadedLevel(hg::OperationInterrupter &)>, SceneAborted_tag> selected_level
                = run_level_selection_scene(window);
            
            if (selected_level.active<SceneAborted_tag>()) {
                continue;
            }
            else {
                assert(selected_level.active<hg::move_function<hg::LoadedLevel(hg::OperationInterrupter &)>>());
            }
            
            variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>>
                game_scene_result = ReloadLevel_tag{};
            while (game_scene_result.active<ReloadLevel_tag>() || game_scene_result.active<move_function<std::vector<InputList>()>>()) {
                try {
                    auto& levelLoadFunction = selected_level.get<hg::move_function<hg::LoadedLevel(hg::OperationInterrupter &)>>();
                    if (game_scene_result.active<move_function<std::vector<InputList>()>>())
                    {
                        game_scene_result = loadAndRunLevel(
                            window,
                            std::move(levelLoadFunction),
                            std::move(game_scene_result.get<move_function<std::vector<InputList>()>>()));
                    }
                    else {
                        game_scene_result = loadAndRunLevel(window, std::move(levelLoadFunction), [&](){return replay;});
                    }
                    assert( game_scene_result.active<GameAborted_tag>()
                         || game_scene_result.active<GameWon_tag>()
                         || game_scene_result.active<ReloadLevel_tag>()
                         || game_scene_result.active<move_function<std::vector<InputList>()>>());
                }
                catch (hg::LuaError const &e) {
                    std::cerr << "There was an error in some lua, the error message was:\n" << e.message << std::endl;
                    return EXIT_FAILURE;
                }
                catch (std::bad_alloc const &) {
                    std::cerr << "oops... ran out of memory ):" << std::endl;
                    return EXIT_FAILURE;
                }
                catch (std::exception const &e) {
                    std::cerr << "A std::exception was caught, e.what() is: " <<e.what() << std::endl;
                    return EXIT_FAILURE;
                }
            }
        }
    }
    catch (WindowClosed_exception const&) {
        //Do nothing, just exit.
    }
    return EXIT_SUCCESS;
}
#if 0
struct {
    typedef void result_type;
    void operator()(LoadedLevel&) const {
        std::cout << "Level Loaded\n";
    }
    void operator()(WindowClosed_tag) const {
        std::cout << "Window Closed\n";
    }
    void operator()(LoadingCanceled_tag) const {
        std::cout << "Loading Canceled\n";
    }
} visitor;
loading_outcome.visit(visitor);
#endif
}
