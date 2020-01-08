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
#include "VulkanRenderer.h"
#include "GLFWApp.h"
#include "GLFWWindow.h"
#include <iostream>

#include "VulkanEngine.h"

namespace hg {
    struct RunGameResultVisitor {
        typedef std::variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>> result_type;
        template<typename Tag>
        std::variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>> operator()(Tag t) const {
            return std::move(t);
        }
    };
    static std::variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>>
        loadAndRunLevel(
            hg::RenderWindow &window,
            GLFWWindow &windowglfw,
            VulkanEngine &eng,
            VulkanRenderer &vkRenderer,
            LoadLevelFunction const &levelLoadingFunction,
            hg::move_function<std::vector<InputList>()> const& replayLoadingFunction = {})
    {
        std::variant<hg::LoadedLevel, LoadingCanceled_tag>
            loading_outcome = load_level_scene(window, windowglfw, levelLoadingFunction, eng, vkRenderer);

        struct {
            hg::RenderWindow &window;
            GLFWWindow &windowglfw;
            VulkanEngine &eng;
            VulkanRenderer &vkRenderer;
            hg::move_function<std::vector<InputList>()> const &replayLoadingFunction;
            typedef std::variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>> result_type;

            std::variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>>
                operator()(LoadedLevel &level) const
            {
                if (replayLoadingFunction) {
                    return std::visit(RunGameResultVisitor{}, run_game_scene(window, windowglfw, eng, vkRenderer, std::move(level), replayLoadingFunction()));
                }
                else {
                    return std::visit(RunGameResultVisitor{}, run_game_scene(window, windowglfw, eng, vkRenderer, std::move(level)));
                }
            }
            std::variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>> operator()(LoadingCanceled_tag) const {
                return GameAborted_tag{};
            }
        } visitor = { window, windowglfw, eng, vkRenderer, replayLoadingFunction };
        return std::visit(visitor, loading_outcome);
    }
    void error_callback_glfw(int error, const char* description)
    {
        std::cerr << "GLFW Error: " << error << ", " << description << "\n";
        throw std::exception("GLFW Error encountered");
    }

std::variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>> runLevel(
    hg::RenderWindow &window,
    GLFWWindow &windowglfw,
    VulkanEngine &vulkanEng,
    VulkanRenderer &vkRenderer,
    std::variant<LoadLevelFunction, SceneAborted_tag> &selected_level,
    move_function<std::vector<InputList>()> &replayLoader)
{
    std::variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>> game_scene_result = ReloadLevel_tag{};
    while (std::holds_alternative<ReloadLevel_tag>(game_scene_result)
        || std::holds_alternative<move_function<std::vector<InputList>()>>(game_scene_result))
    {
        try {
            auto& levelLoadFunction = std::get<LoadLevelFunction>(selected_level);
            if (std::holds_alternative<move_function<std::vector<InputList>()>>(game_scene_result))
            {
                game_scene_result = loadAndRunLevel(
                    window,
                    windowglfw,
                    vulkanEng,
                    vkRenderer,
                    levelLoadFunction,
                    std::move(std::get<move_function<std::vector<InputList>()>>(game_scene_result)));
            }
            else {
                game_scene_result = loadAndRunLevel(window, windowglfw, vulkanEng, vkRenderer, levelLoadFunction, std::move(replayLoader));
            }
            assert(std::holds_alternative<GameAborted_tag>(game_scene_result)
                || std::holds_alternative<GameWon_tag>(game_scene_result)
                || std::holds_alternative<ReloadLevel_tag>(game_scene_result)
                || std::holds_alternative<move_function<std::vector<InputList>()>>(game_scene_result));
        }
        catch (hg::LuaError const &e) {
            std::cerr << "There was an error in some lua, the error message was:\n" << boost::diagnostic_information(e) << std::endl;
            report_runtime_error(window, windowglfw, e);
            return GameAborted_tag{};
        }
    }
    return game_scene_result;
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

        // 'window' is the SDL instance and window
        hg::RenderWindow window(sf::VideoMode(hg::WINDOW_DEFAULT_X, hg::WINDOW_DEFAULT_Y), windowTitle, sf::Style::Titlebar | sf::Style::Resize | sf::Style::Close);
        window.setIcon(window_icon_image.getSize().x, window_icon_image.getSize().y, window_icon_image.getPixelsPtr());
        window.setVerticalSyncEnabled(true);
        window.setFramerateLimit(hg::FRAMERATE);

        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        // windowglfw holds the Vulkan instance
        GLFWWindow windowglfw(hg::WINDOW_DEFAULT_X, /*10*/hg::WINDOW_DEFAULT_Y, windowTitle, NULL, NULL);
        // Set key and mouse presses to be saved until the next poll events
        glfwSetInputMode(windowglfw.w, GLFW_STICKY_KEYS, GLFW_TRUE);
        glfwSetInputMode(windowglfw.w, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);

        glfwSetKeyCallback(windowglfw.w, &GLFWWindow::key_callback);
        // glfwSetWindowUserPointer is not used, since Vulkan requires it and there is only one User Pointer.
        // Using a pointer for both would require edits to Vulkan.
        //glfwSetWindowUserPointer(windowglfw.w, (void *)&windowglfw);

        glfwDefaultWindowHints();
        std::vector<unsigned char> icon;
        boost::push_back(icon, boost::make_iterator_range_n(window_icon_image.getPixelsPtr(), window_icon_image.getSize().x * window_icon_image.getSize().y * 4));
        GLFWimage window_icon_image_glfw{
                gsl::narrow<int>(window_icon_image.getSize().x),
                gsl::narrow<int>(window_icon_image.getSize().y),
                icon.data()
        };
        glfwSetWindowIcon(windowglfw.w, 1, &window_icon_image_glfw);
        glfwShowWindow(windowglfw.w);

        VulkanEngine vulkanEng(*windowglfw.w);
        std::atomic<bool> stopRenderer{false};
        VulkanRenderer vkRenderer;
        std::thread renderThread(
            [&]{
                while (!(stopRenderer && vkRenderer.hasNoKeepAlives())) {
                    //TODO: allow for single-stepping/not re-rendering identical scenes?
                    //maybe by giving each scene a 'waitForRenderReady' function?
                    //or some other design.

                    //TODO: Avoid busy wait, reduce number of calls related to vulkanEng.idle
                    //(only currently alive frames actually need to be checked, not every frame every time)

                    SceneLock lock{vkRenderer.lockScene()};
                    if (lock.shouldDraw()) {
                        //TODO: when !shouldDraw(); wait on a conditional variable for both
                        //stopRenderer and shouldDraw, to avoid wasteful busy wait loop.
                        vulkanEng.drawFrame(vkRenderer);
                    }
                    else {
                        //TODO: don't lockScene during idle???
                        //(would need rework elsewhere too)
                        vulkanEng.idle(vkRenderer);
                    }
                }
            }
        );
        struct CleanupEnforcer final {
            decltype(stopRenderer) &stopRenderer_;
            decltype(renderThread) &renderThread_;
            ~CleanupEnforcer() noexcept {
                stopRenderer_ = true;
                renderThread_.join();
            }
        } CleanupEnforcer_obj{ stopRenderer, renderThread };
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
            std::variant<RunALevel_tag, RunAReplay_tag, Exit_tag> const main_menu_result = run_main_menu(window, windowglfw, vulkanEng, vkRenderer);
            if (std::holds_alternative<Exit_tag>(main_menu_result)) {
                return EXIT_SUCCESS;
            }
            else if (std::holds_alternative<RunAReplay_tag>(main_menu_result)) {
            }
            else {
                assert(std::holds_alternative<RunALevel_tag>(main_menu_result));
            }

            // Run a Replay
            if (std::holds_alternative<RunAReplay_tag>(main_menu_result)) {
                std::string levelName = "";
                while (true) {
                    std::variant<LoadLevelFunction, SceneAborted_tag> selected_level
                        = run_level_selection_scene(window, windowglfw, vulkanEng, vkRenderer, levelName);

                    if (std::holds_alternative<SceneAborted_tag>(selected_level)) {
                        break;
                    }
                    else {
                        assert(std::holds_alternative<LoadLevelFunction>(selected_level));
                    }
                    levelName = std::get<LoadLevelFunction>(selected_level).levelName;
                    move_function<std::vector<InputList>()> replayLoader;
                    std::variant<move_function<std::vector<InputList>()>, SceneAborted_tag> selected_replay
                        = run_replay_selection_scene(window, windowglfw,
                            std::get<LoadLevelFunction>(selected_level).levelName, vulkanEng, vkRenderer);
                    if (std::holds_alternative<SceneAborted_tag>(selected_replay)) {
                        continue;
                    }
                    else {
                        assert(std::holds_alternative<move_function<std::vector<InputList>()>>(selected_replay));
                    }
                    //TODO: Make failure to use std::move here be a compile time error, rather than
                    //      a stack-overflow.
                    //      (requires enhancement to move_function)
                    replayLoader = std::move(std::get<move_function<std::vector<InputList>()>>(selected_replay));

                    std::variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>>
                        game_scene_result = runLevel(window, windowglfw, vulkanEng, vkRenderer, selected_level, replayLoader);

                }
            }

            // Run a Level
            if (std::holds_alternative<RunALevel_tag>(main_menu_result)) {
                std::string levelName = "";
                while (true) {
                    std::variant<LoadLevelFunction, SceneAborted_tag> selected_level
                        = run_level_selection_scene(window, windowglfw, vulkanEng, vkRenderer, levelName);

                    if (std::holds_alternative<SceneAborted_tag>(selected_level)) {
                        break;
                    }
                    else {
                        assert(std::holds_alternative<LoadLevelFunction>(selected_level));
                    }
                    levelName = std::get<LoadLevelFunction>(selected_level).levelName;
                    move_function<std::vector<InputList>()> replayLoader;

                    std::variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>>
                        game_scene_result = runLevel(window, windowglfw, vulkanEng, vkRenderer, selected_level, replayLoader);
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
