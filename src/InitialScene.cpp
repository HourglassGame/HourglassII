#include "InitialScene.h"
#include "RenderWindow.h"
#include "LoadingLevelScene.h"
#include "LoadedLevel.h"
#include "RunningGameScene.h"
#include "GlobalConst.h"
#include "LuaError.h"
#include "move_function.h"
#include "MainMenuScene.h"
#include "ReplayIO.h"
#include "LevelSelectionScene.h"
#include <functional>
#include <iostream>
#include "ReplaySelectionScene.h"
#include "RuntimeErrorScene.h"
#include "GlobalConst.h"

#include "GLFWApp.h"
#include "GLFWWindow.h"

#include "VulkanEngine.h"

namespace hg {
struct RunGameResultVisitor {
    typedef variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>>result_type;
    template<typename Tag>
    variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>> operator()(Tag t) const {
        return std::move(t);
    }
};
static variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>>
    loadAndRunLevel(
        hg::RenderWindow &window,
        VulkanEngine &eng,
        LoadLevelFunction const &levelLoadingFunction,
        hg::move_function<std::vector<InputList>()> const& replayLoadingFunction = {})
{
    hg::variant<hg::LoadedLevel, LoadingCanceled_tag>
              loading_outcome = load_level_scene(window, levelLoadingFunction);
    
    struct {
        hg::RenderWindow &window;
        VulkanEngine &eng;
        hg::move_function<std::vector<InputList>()> const &replayLoadingFunction;
        typedef variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>> result_type;

        variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>>
        operator()(LoadedLevel &level) const
        {
            if (replayLoadingFunction) {
                return run_game_scene(window, eng, std::move(level), replayLoadingFunction()).visit(RunGameResultVisitor{});
            }
            else {
                return run_game_scene(window, eng, std::move(level)).visit(RunGameResultVisitor{});
            }
        }
        variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>> operator()(LoadingCanceled_tag) const {
            return GameAborted_tag{};
        }
    } visitor = {window, eng, replayLoadingFunction};
    return loading_outcome.visit(visitor);
}
void error_callback_glfw(int error, const char* description)
{
    std::cerr << "GLFW Error: " << error << ", " << description << "\n";
    throw std::exception("GLFW Error encountered");
}
int run_hourglassii() {

    try {
        //Create window
        glfwSetErrorCallback(error_callback_glfw);

        GLFWApp glfw;

        auto const windowTitle{ "Hourglass II" };
        sf::Image window_icon_image;

        if (!window_icon_image.loadFromFile("images/HourglassSwirl_64x64.png")) {
            throw std::exception("Couldn't load window icon");
        }

        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWWindow const windowglfw(hg::WINDOW_DEFAULT_X, hg::WINDOW_DEFAULT_Y, windowTitle, NULL, NULL);
        glfwDefaultWindowHints();
        GLFWimage window_icon_image_glfw{
                window_icon_image.getSize().x,
                window_icon_image.getSize().y,
                const_cast<unsigned char *>(reinterpret_cast<unsigned char const*>(window_icon_image.getPixelsPtr()))
        };
        glfwSetWindowIcon(windowglfw.w, 1, &window_icon_image_glfw);
        glfwShowWindow(windowglfw.w);


        hg::RenderWindow window(sf::VideoMode(hg::WINDOW_DEFAULT_X, hg::WINDOW_DEFAULT_Y), windowTitle, sf::Style::Titlebar | /*sf::Style::Resize |*/ sf::Style::Close);
        window.setIcon(window_icon_image.getSize().x, window_icon_image.getSize().y, window_icon_image.getPixelsPtr());
        window.setVerticalSyncEnabled(true);
        window.setFramerateLimit(hg::FRAMERATE);

        VulkanEngine vulkanEng(*windowglfw.w);
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

        while (true) {
            variant<RunALevel_tag, RunAReplay_tag, Exit_tag> const main_menu_result = run_main_menu(window, vulkanEng);
            if (main_menu_result.active<Exit_tag>()) {
                return EXIT_SUCCESS;
            }
            else if (main_menu_result.active<RunAReplay_tag>()) {
            }
            else {
                assert(main_menu_result.active<RunALevel_tag>());
            }
            
            variant<LoadLevelFunction, SceneAborted_tag> selected_level
                = run_level_selection_scene(window);
            
            if (selected_level.active<SceneAborted_tag>()) {
                continue;
            }
            else {
                assert(selected_level.active<LoadLevelFunction>());
            }
            move_function<std::vector<InputList>()> replayLoader;
            if (main_menu_result.active<RunAReplay_tag>()) {
                variant<move_function<std::vector<InputList>()>, SceneAborted_tag> selected_replay
                    = run_replay_selection_scene(window, selected_level.get<LoadLevelFunction>().levelName);
                if (selected_replay.active<SceneAborted_tag>()) {
                    continue;
                }
                else {
                    assert(selected_replay.active<move_function<std::vector<InputList>()>>());
                }
                //TODO: Make failure to use std::move here be a compile time error, rather than
                //      a stack-overflow.
                //      (requires enhancement to move_function)
                replayLoader = std::move(selected_replay.get<move_function<std::vector<InputList>()>>());
            }
            
            variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>>
                game_scene_result = ReloadLevel_tag{};
            while (game_scene_result.active<ReloadLevel_tag>()
                || game_scene_result.active<move_function<std::vector<InputList>()>>())
            {
                try {
                    auto& levelLoadFunction = selected_level.get<LoadLevelFunction>();
                    if (game_scene_result.active<move_function<std::vector<InputList>()>>())
                    {
                        game_scene_result = loadAndRunLevel(
                            window,
                            vulkanEng,
                            levelLoadFunction,
                            std::move(game_scene_result.get<move_function<std::vector<InputList>()>>()));
                    }
                    else {
                        game_scene_result = loadAndRunLevel(window, vulkanEng, levelLoadFunction, std::move(replayLoader));
                    }
                    assert( game_scene_result.active<GameAborted_tag>()
                         || game_scene_result.active<GameWon_tag>()
                         || game_scene_result.active<ReloadLevel_tag>()
                         || game_scene_result.active<move_function<std::vector<InputList>()>>());
                }
                catch (hg::LuaError const &e) {
                    std::cerr << "There was an error in some lua, the error message was:\n" << boost::diagnostic_information(e) << std::endl;
                    report_runtime_error(window, e);
                    game_scene_result = GameAborted_tag{};
                }
            }
        }
    }
    catch (WindowClosed_exception const&) {
        //Do nothing, just exit.
    }
    catch (std::bad_alloc const &) {
        //report_out_of_memory();
        std::cerr << "oops... ran out of memory ):" << std::endl;
        return EXIT_FAILURE;
    }
    catch (std::exception const &e) {
        std::cerr << "A std::exception was caught, e.what() is: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        std::cerr << "An unknown exception was caught " << std::endl;
        return EXIT_FAILURE;
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
