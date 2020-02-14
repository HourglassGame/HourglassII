#include "InitialScene.h"
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
            GLFWWindow &windowglfw,
            VulkanEngine &eng,
            VulkanRenderer &vkRenderer,
            LoadLevelFunction const &levelLoadingFunction,
            hg::move_function<std::vector<InputList>()> const& replayLoadingFunction = {})
    {
        std::variant<hg::LoadedLevel, LoadingCanceled_tag>
            loading_outcome = load_level_scene(windowglfw, levelLoadingFunction, eng, vkRenderer);

        struct {
            GLFWWindow &windowglfw;
            VulkanEngine &eng;
            VulkanRenderer &vkRenderer;
            hg::move_function<std::vector<InputList>()> const &replayLoadingFunction;
            typedef std::variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>> result_type;

            std::variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>>
                operator()(LoadedLevel &level) const
            {
                if (replayLoadingFunction) {
                    return std::visit(RunGameResultVisitor{}, run_game_scene(/*window, */windowglfw, eng, vkRenderer, std::move(level), replayLoadingFunction()));
                }
                else {
                    return std::visit(RunGameResultVisitor{}, run_game_scene(/*window, */windowglfw, eng, vkRenderer, std::move(level)));
                }
            }
            std::variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>> operator()(LoadingCanceled_tag) const {
                return GameAborted_tag{};
            }
        } visitor = { /*window,*/ windowglfw, eng, vkRenderer, replayLoadingFunction };
        return std::visit(visitor, loading_outcome);
    }
    void error_callback_glfw(int error, const char* description)
    {
        std::cerr << "GLFW Error: " << error << ", " << description << "\n";
        throw std::exception("GLFW Error encountered");
    }

std::variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>> runLevel(
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
                    //window,
                    windowglfw,
                    vulkanEng,
                    vkRenderer,
                    levelLoadFunction,
                    std::move(std::get<move_function<std::vector<InputList>()>>(game_scene_result)));
            }
            else {
                game_scene_result = loadAndRunLevel(/*window,*/ windowglfw, vulkanEng, vkRenderer, levelLoadFunction, std::move(replayLoader));
            }
            assert(std::holds_alternative<GameAborted_tag>(game_scene_result)
                || std::holds_alternative<GameWon_tag>(game_scene_result)
                || std::holds_alternative<ReloadLevel_tag>(game_scene_result)
                || std::holds_alternative<move_function<std::vector<InputList>()>>(game_scene_result));
        }
        catch (hg::LuaError const &e) {
            std::cerr << "There was an error in some lua, the error message was:\n" << boost::diagnostic_information(e) << std::endl;
            report_runtime_error(/*window, */windowglfw, vulkanEng, vkRenderer, e);
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

        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        // windowglfw holds the Vulkan instance
        GLFWWindow windowglfw(hg::WINDOW_DEFAULT_X, /*10*/hg::WINDOW_DEFAULT_Y, windowTitle, nullptr, nullptr);
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
                //TODO!!!{
//Render thread needs to rerun renderer whenever
//    Scene wants to re-render (and it hasn't already done so)
// OR Swap-chain recreated/renderTarget size changed/vulkan engine reset/etc
//
//(Idle process waits for previously rendered frames to complete, and
// frees any resources those frames were using)
//Render thread needs to run idle process whenever
//   There are pending frames, but there is no need to re-run the renderer.


//Render thread should exit once told to do so, and once all pending frames have been
//flushed.


//Render thread can block pending (possibly more than one of...):
//(not necessarily a hard block)
// Scene wanting to re-render
// Frames needing to complete to flush them in the idle process
// Frames needing to complete so that the relevant frame data becomes available to render a new frame
// Rendering stopped
// Frame being available
//}


//TODO:
// How exactly should the 'Wants to re-render' flag work?
// In the common case, there is some data attached to the flag,
// but if Render Thread is not aware of this data, there is probably
// a race condition between Render Thread saying that the latest data has been renderered
// and the scene thread (or wherever) updating the data and setting the re-render flag.

//Possibly just have an atomic bool.
//The contract can be that when it is set to true,
//the renderer is guaranteed to run at least once, using the state of the world
//at (or after) the time that the flag was set to true (appropriate memory order flag 
// could do this?)
//The render thread could then just safely set the boolean to false just before running the renderer.
//Either the renderer will run twice on the same data (with the second render fulfilling the contract for sure; if the setter sets the bool to true afterwards),
//or the setter set the bool before the renderer did, and so the first render fulfills the contract)


//Render thread needs to be able to tell the main thread
//when it is and is not safe to destory a scene

//TODO:
// Include a mode that always re-renders, even if not asked for (for performance testing)?
#if 0
                //render thread state machine?
                //
                //FrameResourcesBecameAvailable event == FrameFinishedRendering == exists f: vkGetFenceStatus(logicalDevice.device, inFlightFences[f].fence)
                //ThereArePendingFrames == AllFrameFinishedRendering == forall f: vkGetFenceStatus(logicalDevice.device, inFlightFences[f].fence)
                if RunRenderer then
                    if FrameResourcesAvailable then
                        if shouldRerenderScene then
                            Add something to handle swapchain recreation here ?
                            RerenderScene; FrameResourcesAvailable = AreFrameResourcesStillAvailable()
                        else
                            while await event
                                | SceneWantsRerender->shouldRerenderScene = true break;
                | EngineWantsRerender->shouldRerenderScene = true break;
                | StopRenderer->RunRenderer = false; break;
                    else
                        while await event with
                            | FrameResourcesBecameAvailable->FrameResourcesAvailable = true break;
                | StopRenderer->RunRenderer = false; break;
                | SceneWantsRerender->shouldRerenderScene = true; break;
                | EngineWantsRerender->shouldRerenderScene = true break;
                end
                else
                    while ThereArePendingFrames
                        waitForPendingFrames
                        end



                        await event with
                        | SceneWantsToRender ->
                        await event with

                        |


#endif


#if 0
                        //THIS IS THE ACTUALLY GOOD DESIGN:

                        //Possible calls

                        //TryDrawFrame
                        // Prerequisites
                        //  Current Frame inFlightFence must have already been signaled
                        //  (indicating that the previous in-flight frame for the index that will be reused
                        //   has already compelted)
                        //  stopRenderer must be false
                        //  There must be a scene (i.e. the Game thread must be somewhere
                        //   betweeen StartScene and EndScene, and any concurrent calls to
                        //   EndScene must be blocked until after TryDrawFrame has completed
                        //  Either scene must have requested a re-draw, or the swap-chain must have been recreated
                        //  since the last re-draw
                        //  Any prior swap-chain size changes must have already be handled.
                        //   (i.e. a failure here must not loop back to calling this again, without an intermediate call to "RecreateSwapChain")
                        //  The latest swapchain framebuffer must not be zero sized
                        // Notes
                        //  May fail if vkAcquireNextImageKHR or vkQueuePresentKHR fail and indicate that the swapchain must be recreated
                        //  (or if a direct check notices that the window size has changed).

                        //RecreateSwapChain
                        // Prerequisites
                        //  stopRenderer must be false
                        //  There must be a scene (i.e. the Game thread must be somewhere
                        //   betweeen StartScene and EndScene, and any concurrent calls to
                        //   EndScene must be blocked until after RecreateSwapChain has completed
                        //  There must be a need to recreate the swap chain
                        //   (detected either via a failure when calling TryDrawFrame, or
                        ///   direct detection of window size changes (need to support both detection methods,
                        //    since the window being resized should trigger a redraw, even if the Scene does not
                        //    explicitly request it in order to trigger a TryDrawFrame))

                        //Clear frame keep-alive
                        // Prerequisites
                        //  Must be at least one in-flight frame that has not yet been cleaned up; and
                        //  which has ended (inFlightFence signalled)
                        //  SceneKeepAlives (etc.) must have somehow been protected
                        //   from concurrent updates by StartScene/EndScene in the Game thread

                        //Idle/Wait for update
                        // Prerequisites
                        //  Must not already have met prerequisites for any other calls
                        //  Must have set up interruptions for all prerequisite conditions that could change

                        //Exit
                        // stopRenderer must be true
                        // There must be no frames that have not completed rendering (aka hasNoKeepAlives)
                        //   (if there are, Clear frame keep-alive should be called instead)



                        //Events
                        // Current Frame inFlightFence must have already been signaled
                        // stopRenderer
                        // SceneExists (and is locked; somewhat as a linked event?)
                        // SceneRequestedRedraw
                        // Framebuffer Size Changed
                        // 


                        //await (StopRenderer | SceneExists) A
                        //|SceneExists ->
                        //    await (StopRenderer | SceneEnded | SceneRequestedRender | SceneShouldRender | FramebufferResized)
                        //    | StopRenderer -> return
                        //    | SceneEnded -> goto A
                        //    | SceneRequestedRender | SceneShouldRender ->
                        //        SceneShouldRender=false; TryDoRender
                        //        | RenderFinished -> AddCurrentFrameToUsedFrameData; goto B
                        //        | NeedsNewSwapchain -> goto K
                        //    | FramebufferResized(newWidth, newHeight) -> :K
                        //        if newWidth || newHeight == 0 goto F
                        //|StopRenderer -> return


                        //F: await 





                        //Initially no frameData is in use and no keepAlives are present

                        //await (StopRenderer



                        //TODO: Avoid blocking on vkAcquireNextImageKHR
                        //      (do this by passing a Fence to the call, and subsequently waiting on the fence at the same time as waiting on the other fences??)
                        //      (I don't think this would work, I'm pretty sure the fence only gets signalled if the call to vkAcquireNextImageKHR didn't time out)
                        //      (It might be necessary to do some polling here)

                        //Possibly add another thread that just blocks on vulkan
                        //and signal it (via vkQueueSubmit?) when non-vulkan events signal
                        //This would avoid the need to poll two different signal sources
                        let rendererShouldStop = false
                        let frameBufferSize = { 0, 0 }
                        let sceneShouldRender = false
                        while !rendererShouldStop && framesInUse
                            if !rendererShouldStop
                                let toAwaitList = [stopRenderer]
                                toAwaitList += FrameBufferResized(frameBufferSize)
                                if (exists frameNotInUse)
                                    toAwaitList += SceneRequestedReRender
                                    end
                                    for frameDataInUse in framesInUse
                                        toAwaitList += FrameFinishedRendereing(frameDataInUse)

                                        await toAwaitList
                                        | FrameFinishedRendering(frame) ->
                                        framesInUse -= frame
                                        | FrameBufferResized(newWidth, newHeight) ->
                                        frameBufferSize = { newWidth, newHeight }
                                        engine.UpdateSwapChain(frameBufferSize) //Engine internally locks scene and calls updateSwapChain on scene if it exists
                                        sceneShouldRender = true
                                        //TODO: Additionally check for glfwSetWindowRefreshCallback
                                        // using logic pretty much identical to FrameBufferResized
                                        | SceneRequestedReRender ->
                                        sceneShouldRender = true
                                        | StopRenderer ->
                                        rendererShouldStop = true

                                        if !rendererShouldStop
                                            if sceneShouldReRender&& exists frameNotInUse
                                                frameBufferSize = engine.DrawScene(frameNotInUse) //Engine internally locks scene and only draws if the scene actually exists
                                                                                                  //If the scene does not exist, it means that the scene that requested
                                                                                                  //rendering no longer exists, so it is safe to make no further attempt to render that scene
                                                sceneShouldRender = false
#endif
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
                        //lock.l.unlock();//DONT JUST DO THIS!
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
            std::variant<RunALevel_tag, RunAReplay_tag, Exit_tag> const main_menu_result = run_main_menu(windowglfw, vulkanEng, vkRenderer);
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
                        = run_level_selection_scene(windowglfw, vulkanEng, vkRenderer, levelName);

                    if (std::holds_alternative<SceneAborted_tag>(selected_level)) {
                        break;
                    }
                    else {
                        assert(std::holds_alternative<LoadLevelFunction>(selected_level));
                    }
                    levelName = std::get<LoadLevelFunction>(selected_level).levelName;
                    move_function<std::vector<InputList>()> replayLoader;
                    std::variant<move_function<std::vector<InputList>()>, SceneAborted_tag> selected_replay
                        = run_replay_selection_scene(windowglfw,
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
                        game_scene_result = runLevel(windowglfw, vulkanEng, vkRenderer, selected_level, replayLoader);

                }
            }

            // Run a Level
            if (std::holds_alternative<RunALevel_tag>(main_menu_result)) {
                std::string levelName = "";
                while (true) {
                    std::variant<LoadLevelFunction, SceneAborted_tag> selected_level
                        = run_level_selection_scene(windowglfw, vulkanEng, vkRenderer, levelName);

                    if (std::holds_alternative<SceneAborted_tag>(selected_level)) {
                        break;
                    }
                    else {
                        assert(std::holds_alternative<LoadLevelFunction>(selected_level));
                    }
                    levelName = std::get<LoadLevelFunction>(selected_level).levelName;
                    move_function<std::vector<InputList>()> replayLoader;

                    std::variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>>
                        game_scene_result = runLevel(windowglfw, vulkanEng, vkRenderer, selected_level, replayLoader);
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
