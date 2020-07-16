#include "RunningGameScene.h"
#include "VulkanRenderTarget.h"

#include "Scene.h"
#include "Hg_Input.h"
#include "Inertia.h"
#include "LoadedLevel.h"
#include "ReplayIO.h"
#include "hg/TimeEngine/TimeEngine.h"
#include "hg/GlobalConst.h"
#include <tbb/task_group.h>
#include <boost/thread/future.hpp>
#include <cassert>
#include <fstream>
#include "hg/TimeEngine/ArrivalDepartures/ObjectPtrList.h"
#include "hg/TimeEngine/ArrivalDepartures/ObjectListTypes.h"
#include "hg/TimeEngine/TimeEngine.h"
#include "hg/TimeEngine/PlayerVictoryException.h"
#include "Hg_Input.h"
#include "hg/TimeEngine/Level.h"
#include "Inertia.h"
#include "hg/TimeEngine/Frame.h"
#include "hg/Util/TestDriver.h"
#include "ReplayIO.h"
#include "LayeredCanvas.h"
#include "ResourceManager.h"
#include "hg/Util/Maths.h"
#include "hg/Util/async.h"

#include "AudioGlitzManager.h"

#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/adaptor/sliced.hpp>
#include <boost/range/algorithm/max_element.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/irange.hpp>
#include <boost/range/istream_range.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/thread/future.hpp>
#include <boost/utility/result_of.hpp>
#include <chrono>

#include <boost/function.hpp>

#include <functional>

#include <tbb/concurrent_queue.h>

#include <fstream>

#include <boost/bind.hpp>

#include <cstddef>
#include <utility>

#include <cmath>

#include "hg/PhysicsEngine/TriggerSystem/DirectLuaTriggerSystem.h"
#include "hg/TimeEngine/LevelLoader.h"
#include "hg/Util/move_function.h"
#include <memory>
#include "sfRenderTargetCanvas.h"
#include "hg/Util/Maths.h"

#include "Scene.h"
#include "InitialScene.h"

#include "InitialScene.h"
#include "LoadingLevelScene.h"
#include "LoadedLevel.h"
#include "PostLevelScene.h"
#include <functional>

#include "LoadingLevelScene.h"
#include <tbb/task_group.h>
#include <boost/thread/future.hpp>
#include <variant>
#include "GameDisplayHelpers.h"

namespace hg {

UIFrameState runStep(
    hg::TimeEngine &timeEngine,
    GLFWWindow &windowglfw,
    std::size_t relativeGuyIndex,
	hg::Input const &input,
	bool const paused,
    hg::Inertia &inertia,
    hg::TimeEngine::RunResult const &waveInfo,
    bool const runningFromReplay,
    bool const frameRun,
    std::vector<std::vector<GuyFrameData>> &run_game_scene_guyFrameData,
    std::vector<std::vector<int>> &run_game_scene_frameGuyData);

void saveReplayLog(std::ostream &toAppendTo, hg::InputList const &toAppend);
void generateReplay();
struct AwaitingInputState {
    
};

struct RunningLevelState {
    
};

ActivePanel const getActivePanel(GLFWWindow &windowglfw)
{
    ActivePanel mousePanel = ActivePanel::NONE;
    // Todo, possibly include xFill and yFill.
    double mouseX, mouseY;
    glfwGetCursorPos(windowglfw.w, &mouseX, &mouseY);
    int width, height;
    glfwGetWindowSize(windowglfw.w, &width, &height);

    if (mouseX > width*(hg::UI_DIVIDE_X))
    {
        if (mouseY <= height*(hg::UI_DIVIDE_Y))
        {
            mousePanel = ActivePanel::WORLD;
        }
        else if (mouseY <= (height*((hg::UI_DIVIDE_Y) + (hg::G_TIME_Y + hg::G_TIME_HEIGHT)*(1. - hg::UI_DIVIDE_Y))))
        {
            if (mouseY >= (height*((hg::UI_DIVIDE_Y) + hg::G_TIME_Y*(1. - hg::UI_DIVIDE_Y))))
            {
                mousePanel = ActivePanel::GLOBAL_TIME;
            }
        }
        else if (mouseY <= (height*((hg::UI_DIVIDE_Y) + (hg::P_TIME_Y + hg::P_TIME_HEIGHT)*(1. - hg::UI_DIVIDE_Y))))
        {
            if (mouseY >= (height*((hg::UI_DIVIDE_Y) + hg::P_TIME_Y*(1. - hg::UI_DIVIDE_Y))))
            {
                mousePanel = ActivePanel::PERSONAL_TIME;
            }
        }
    }
    return mousePanel;
}

std::variant<
    GameAborted_tag,
    GameWon_tag,
    ReloadLevel_tag,
    move_function<std::vector<hg::InputList>()>
>
run_game_scene(
    GLFWWindow &windowglfw,
    VulkanEngine &eng,
    VulkanRenderer &vkRenderer,
    LoadedLevel &&loadedLevel,
    std::vector<hg::InputList> const& replay)
{
    RunningGameSceneRenderer renderer(
        eng.physicalDevice,
        eng.device.h(),
        eng.surface.surface,
        eng.renderPass.h(),
        //eng.graphicsPipeline.graphicsPipeline,
        //eng.pipelineLayout.pipelineLayout,
        //eng.descriptorSetLayout.descriptorSetLayout,
        eng.swapChain.extent(),
        eng.device.graphicsQ()
    );
    vkRenderer.StartScene(renderer);
    struct RendererCleanupEnforcer final {
        decltype(vkRenderer) &vkRenderer_;
        ~RendererCleanupEnforcer() noexcept {
            vkRenderer_.EndScene();
        }
    } RendererCleanupEnforcer_obj{ vkRenderer };
#if 0
    std::atomic<bool> stopRenderer{false};
    //TODO
    std::thread renderThread([&renderer, &eng, &stopRenderer]{
        while(!stopRenderer) {
            try{
                eng.drawFrame(renderer);
            }
            catch(std::system_error const &e) {
                std::cerr << "renderThreadSystemError: " << boost::diagnostic_information(e) << e.code() << " " << e.code().message() << "\n\n" << std::flush;
            }
            catch(std::exception const &e) {
                std::cerr << "renderThreadError: " << boost::diagnostic_information(e) << "\n" << std::flush;
            }
            catch(...){
                std::cerr << "unknownRenderThreadError\n" << std::flush;
            }
        }
    });
    struct RendererCleanupEnforcer final {
        decltype(stopRenderer) &stopRenderer_;
        //decltype(renderThread) &renderThread_;
        ~RendererCleanupEnforcer() noexcept {
            stopRenderer_ = true;
            //renderThread_.join();
        }
    } RendererCleanupEnforcer_obj{ stopRenderer/*, renderThread */};

#endif

    std::vector<InputList> receivedInputs;

    auto frameStartTime{std::chrono::steady_clock::now()};
    TimeEngine const initialTimeEngine(loadedLevel.timeEngine);

    auto audioPlayingState = AudioPlayingState(loadedLevel.resources.sounds);
    auto audioGlitzManager = AudioGlitzManager();

    hg::TimeEngine &timeEngine = loadedLevel.timeEngine;
    hg::LevelResources const &levelResources = loadedLevel.resources;

    std::vector<std::vector<GuyFrameData>> run_game_scene_guyFrameData;
    std::vector<std::vector<int>> run_game_scene_frameGuyData;

    for (int i = 0; i < timeEngine.getTimelineLength(); ++i) {
        run_game_scene_frameGuyData.push_back(std::vector<int>());
    }

    enum class RunState { AWAITING_INPUT, RUNNING_LEVEL };
    RunState state(RunState::AWAITING_INPUT);
    bool paused = false;
    bool frameRun = false;
    bool runNextPausedFrame = false;
    int waitingForWavePress = 0;
    bool waitingForWave = false;
    bool levelLost = false;

    hg::Input input;
    input.setTimelineLength(timeEngine.getTimelineLength());
    hg::Inertia inertia;
    std::size_t relativeGuyIndex = 0;

    std::vector<hg::InputList>::const_iterator currentReplayIt(replay.begin());
    std::vector<hg::InputList>::const_iterator currentReplayEnd(replay.end());
    //Saves a replay continuously, but in a less nice format.
    //Gets rewritten whenever the level is reset.
    //Useful for tracking down crashes.
    std::ofstream replayLogOut("replayLogOut");

    auto interrupter = std::make_unique<hg::OperationInterrupter>();
    boost::future<hg::TimeEngine::RunResult> futureRunResult;

    struct TimeEngineCleanupEnforcer final {
        decltype(interrupter) &interrupter_;
        decltype(futureRunResult) &futureRunResult_;
        ~TimeEngineCleanupEnforcer() {
            if (interrupter_) interrupter_->interrupt();
            if (futureRunResult_.valid()) futureRunResult_.wait();
        }
    } TimeEngineCleanupEnforcer_obj{interrupter, futureRunResult};
    bool runningFromReplay(false);

    while (true) {
        glfwPollEvents();

        switch (state) {
        case RunState::AWAITING_INPUT:
        {
            hg::InputList inputList;
            if (currentReplayIt != currentReplayEnd) {
                if (!paused) {
                    inputList = *currentReplayIt;
                    ++currentReplayIt;
                }
                runningFromReplay = true;
            }
            else {
                int width, height;
                glfwGetWindowSize(windowglfw.w, &width, &height);

                hg::Wall const &wall(timeEngine.getWall());
                double xScale = (width*(1. - hg::UI_DIVIDE_X)) / wall.roomWidth();
                double yScale = (height*hg::UI_DIVIDE_Y) / wall.roomHeight();
                double scalingFactor(std::min(xScale, yScale));
                double xFill = scalingFactor / xScale;
                double yFill = scalingFactor / yScale;
                int mouseOffX = static_cast<int>(width*(hg::UI_DIVIDE_X + (1. - xFill)*(1. - hg::UI_DIVIDE_X) / 2.));
                int mouseOffY = static_cast<int>(height*((1. - yFill)*hg::UI_DIVIDE_Y / 2.));
                int timelineOffset = static_cast<int>(width*(hg::UI_DIVIDE_X + hg::TIMELINE_PAD_X));
                int timelineWidth = static_cast<int>(width*((1.f - hg::UI_DIVIDE_X) - 2.f*hg::TIMELINE_PAD_X));
                int personalTimelineWidth = (timeEngine.getReplayData().size() > 0) ? std::min(timelineWidth, static_cast<int>(timelineWidth*timeEngine.getReplayData().size() / timeEngine.getTimelineLength())) : timelineWidth;
                
                ActivePanel mousePanel = getActivePanel(windowglfw);

                input.updateState(/*window.getInputState(), */windowglfw, mousePanel, waitingForWave,
                    timelineOffset, timelineWidth, personalTimelineWidth,
                    timeEngine.getReplayData().size(),
                    mouseOffX, mouseOffY, 1. / scalingFactor, frameRun);
                inputList = input.AsInputList();
                runningFromReplay = false;
            }
            relativeGuyIndex = inputList.getRelativeGuyIndex();
            interrupter = std::make_unique<hg::OperationInterrupter>();

            if (levelLost || inputList.getGuyInput().getActionPause() || (paused && !(runNextPausedFrame || inputList.getGuyInput().getPauseActionTaken()))) {
                frameRun = false;
                futureRunResult =
                    async(
                        [&timeEngine] {
                    return timeEngine.getPrevRunResult(); });
            }
            else {
                frameRun = true;
                if (paused && inputList.getGuyInput().getPauseActionTaken()) {
                    runNextPausedFrame = true;
                }
                else {
                    runNextPausedFrame = false;
                }

                saveReplayLog(replayLogOut, inputList);
                receivedInputs.push_back(inputList);
                futureRunResult =
                    async(
                        [inputList, &timeEngine, relativeGuyIndex, &interrupter] {
                    return timeEngine.runToNextPlayerFrame(std::move(inputList), relativeGuyIndex, *interrupter); });
            }

            state = RunState::RUNNING_LEVEL;
            break;
        }

        case RunState::RUNNING_LEVEL:
        {
            if (glfwWindowShouldClose(windowglfw.w)) {
                //window.close();
                throw WindowClosed_exception{};
            }

            if (windowglfw.hasLastKey()) {
                int key = windowglfw.useLastKey();
                //Leave level
                if (key == GLFW_KEY_ESCAPE) {
                    return GameAborted_tag{};
                }
                //Restart
                if (key == GLFW_KEY_R) {
                    return ReloadLevel_tag{};
                }
                //Load replay
                if (key == GLFW_KEY_L) {
                    return move_function<std::vector<InputList>()>([] {return loadReplay("replay"); });
                }
                //Interrupt replay and begin Playing
                if (key == GLFW_KEY_C) {
                    currentReplayIt = replay.end();
                    currentReplayEnd = replay.end();
                }
                //Save replay
                if (key == GLFW_KEY_K) {
                    saveReplay("replay", receivedInputs);
                }
                //Generate a replay from replayLogIn
                if (key == GLFW_KEY_G) {
                    generateReplay();
                }
                //Pause
                if (key == GLFW_KEY_P) {
                    paused = !paused;
                }
            }

            //Wait for wave
            // This would be better off in Hg_Input.cpp, but for now it interacts with too many other things.
            if (!runningFromReplay) {
                if (glfwGetKey(windowglfw.w, GLFW_KEY_V) == GLFW_PRESS) {
                    if (waitingForWavePress >= 0) {
                        waitingForWave = true;
                        if (waitingForWavePress < 2) {
                            waitingForWavePress += 1;
                        }
                    }
                    else {
                        waitingForWave = false;
                    }
                }
                else {
                    waitingForWavePress = 0;
                    waitingForWave = false;
                }
            }

            if (futureRunResult.wait_for(boost::chrono::duration<double>(1.f / (hg::FRAMERATE))) == boost::future_status::ready) {
                if (glfwGetKey(windowglfw.w, GLFW_KEY_PERIOD) == GLFW_PRESS) {
                    inertia.save(mousePosToFrameID(windowglfw, timeEngine), TimeDirection::FORWARDS);
                }
                if (glfwGetKey(windowglfw.w, GLFW_KEY_COMMA) == GLFW_PRESS) {
                    inertia.save(mousePosToFrameID(windowglfw, timeEngine), TimeDirection::REVERSE);
                }
                if (glfwGetKey(windowglfw.w, GLFW_KEY_SLASH) == GLFW_PRESS) {
                    inertia.reset();
                }
                try {
                    assert(futureRunResult.get_state() != boost::future_state::uninitialized);
                    auto const waveInfo{futureRunResult.get()};
                    auto uiFrameState{
                        runStep(
                            timeEngine,
                            windowglfw,
                            relativeGuyIndex,
							input,
							paused,
                            inertia,
                            waveInfo,
                            runningFromReplay,
                            frameRun,
                            run_game_scene_guyFrameData,
                            run_game_scene_frameGuyData)
                    };
                    interrupter.reset();

                    if (waveInfo.paradoxPressure >= hg::PARADOX_PRESSURE_MAX) {
                        levelLost = true;
                    }

                    if (waitingForWavePress >= 2 && uiFrameState.guyFrameUpdated) {
                        waitingForWavePress = -1;
                    }

                    //TODO: Possbily sync with Graphics?, via
                    //Vulkan Timestamp Queries:
                    //https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#queries-timestamps

                    //TODO: Support slowing down/stopping sound if game is playing slowly, or speeding up sound
                    //      if game is going fast; Support proper audio rendering of timeline scrubbing.
                    //      Somehow support proper sound in Pause Time.
                    //      etc.
                    if (!(paused || levelLost)) {
                        PlayAudioGlitz(
                            getGlitzForDirection(timeEngine.getFrame(uiFrameState.drawnFrame)->getView(), uiFrameState.drawnTimeDirection),
                            audioPlayingState,
                            audioGlitzManager,
                            static_cast<int>(uiFrameState.guyIndex)
                        );
                    }
                    //TODO: Play UI Sounds
#if 0
                    drawFrame(
                        window.getRenderTarget(),
                        eng,
                        //timeEngine,
                        uiFrameState,

                        levelResources,
                        wallImage,
                        positionColoursImage
                    );
#endif
                    renderer.setUiFrameState(std::move(uiFrameState));
                    /*
                    renderer.uiFrameState = &uiFrameState;
                    renderer.timeEngine = &timeEngine;
                    */
                    //eng.drawFrame(renderer);

                    if ((waitingForWave) || (glfwGetKey(windowglfw.w, GLFW_KEY_F) == GLFW_PRESS)) {
                        //window.setFramerateLimit(0);
                        //window.setVerticalSyncEnabled(false);
                        frameStartTime = std::chrono::steady_clock::now();
                    }
                    else {
                        //window.setFramerateLimit(hg::FRAMERATE);
                        //window.setVerticalSyncEnabled(true);
                        
                        //TODO: Avoid busy-wait
                        while (true) {
                            auto const t{std::chrono::steady_clock::now()};
                            if (t >= frameStartTime+std::chrono::duration<double>(1./FRAMERATE)) {
                                frameStartTime = t;
                                break;
                            }
                        }
                    }
                    //window.display();
                    
                    state = RunState::AWAITING_INPUT;
                }
                catch (hg::PlayerVictoryException const &) {
                    //TODO: Leave this scene properly before entering post_level_scene.
                    //(to avoid excessive memory use)
                    saveReplay("replay", receivedInputs); // Save replay on win
                    vkRenderer.EndScene();
                    run_post_level_scene(/*window, */windowglfw, eng, vkRenderer, initialTimeEngine, loadedLevel);
                    //TODO -- Check run_post_level_scene return values (once it gets return values)
                    return GameWon_tag{};
                }
                
            }
            break;
        }
        }
    }
    assert(false && "should be unreachable");
}

UIFrameState runStep(
    hg::TimeEngine &timeEngine,
    GLFWWindow &windowglfw,
    std::size_t relativeGuyIndex,
	hg::Input const &input,
	bool const paused,
    hg::Inertia &inertia,
    hg::TimeEngine::RunResult const &waveInfo,
    bool const runningFromReplay,
    bool const frameRun,
    std::vector<std::vector<GuyFrameData>> &run_game_scene_guyFrameData,
    std::vector<std::vector<int>> &run_game_scene_frameGuyData)
{
    hg::FrameID drawnFrame;
    hg::TimeDirection drawnTimeDirection{hg::TimeDirection::INVALID};
    Pickups pickups;
    bool shouldDrawInventory{false};
    bool guyFrameUpdated(false);
    bool const shouldDrawGuyPositionColours{(glfwGetKey(windowglfw.w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)};
    std::size_t guyIndex = timeEngine.getGuyFrames().size() - 2 - relativeGuyIndex;

    ActivePanel mousePanel = getActivePanel(windowglfw);
    if ((glfwGetKey(windowglfw.w, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) && mousePanel != ActivePanel::PERSONAL_TIME) {
        drawnFrame = mousePosToFrameID(windowglfw, timeEngine);
        drawnTimeDirection = TimeDirection::FORWARDS;
    }
    else {
        if ((glfwGetKey(windowglfw.w, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) && mousePanel == ActivePanel::PERSONAL_TIME) {
            guyIndex = mousePosToGuyIndex(windowglfw, timeEngine);
            relativeGuyIndex = timeEngine.getGuyFrames().size() - 2 - guyIndex;
        }

        if (!timeEngine.getGuyFrames()[guyIndex].empty()) {
            size_t frameChoice = guyIndex % timeEngine.getGuyFrames()[guyIndex].size();
            hg::Frame *guyFrame{ 0 };
            for (hg::Frame *frame : timeEngine.getGuyFrames()[guyIndex]) {
                if (frameChoice == 0) {
                    guyFrame = frame;
                    break;
                }
                --frameChoice;
            }

            hg::GuyOutputInfo const &currentGuy(findCurrentGuy(guyFrame->getView().getGuyInformation(), guyIndex));

            drawnTimeDirection = currentGuy.getTimeDirection();
            drawnFrame = hg::FrameID(guyFrame);
            inertia.save(drawnFrame, drawnTimeDirection);

            shouldDrawInventory = true;
            pickups = currentGuy.getPickups();
        }
        else {
            if (frameRun) {
                inertia.run();
            }
            if (hg::FrameID const inertialFrame(inertia.getFrame()); inertialFrame.isValidFrame()) {
                drawnFrame = inertialFrame;
                drawnTimeDirection = inertia.getTimeDirection();
            }
            else {
                drawnFrame = mousePosToFrameID(windowglfw, timeEngine);
                drawnTimeDirection = TimeDirection::FORWARDS;
            }
        }
    }

    // This has potentially terrible performance, and the work may be duplicated elsewhere
    for (hg::FrameUpdateSet const &updateSet : waveInfo.updatedFrames) {
        for (Frame *frame : updateSet) {
            if (drawnFrame.getFrameNumber() == getFrameNumber(frame)) {
                guyFrameUpdated = true;
                break;
            }
        }
        if (guyFrameUpdated) {
            break;
        }
    }

    // New hacky system to update guyFrameData based on the frames that changed.
    // Should be replaced alongside the graphics rework.
    std::vector<std::set<int> > const &frameGuys{ timeEngine.getFrameGuys() };

    for (hg::FrameUpdateSet const &updateSet : waveInfo.updatedFrames) {
        for (Frame *frame : updateSet) {
            std::vector<int> const &changedGuyIndicies = run_game_scene_frameGuyData[getFrameNumber(frame)];
            for (size_t i = 0; i < changedGuyIndicies.size(); ++i) {
                int gi = changedGuyIndicies[i];
                if (gi < run_game_scene_guyFrameData.size()) {
                    for (size_t j = 0; j < run_game_scene_guyFrameData[gi].size(); ++j) {
                        if (run_game_scene_guyFrameData[gi][j].frameNumber == getFrameNumber(frame)) {
                            run_game_scene_guyFrameData[gi][j] = run_game_scene_guyFrameData[gi].back();
                            run_game_scene_guyFrameData[gi].pop_back();
                            --j;
                        }
                    }
                }
            }
            run_game_scene_frameGuyData[getFrameNumber(frame)].clear();
        }
    }

    int maxNewGuyIndex = timeEngine.getGuyFrames().size() - 1; // The last guy arrival does not have input
    for (hg::FrameUpdateSet const &updateSet : waveInfo.updatedFrames) {
        for (Frame *frame : updateSet) {
            std::set<int> const &changedGuyIndicies = frameGuys[getFrameNumber(frame)];
            std::vector<int> &rgs_frameGuyData = run_game_scene_frameGuyData[getFrameNumber(frame)];
			for (std::set<int>::iterator it = changedGuyIndicies.begin(); it != changedGuyIndicies.end(); ++it) {
                int gi = *it;
                if (gi < maxNewGuyIndex) {
                    while (run_game_scene_guyFrameData.size() <= gi) {
                        run_game_scene_guyFrameData.push_back(std::vector<GuyFrameData>());
                    }
                    run_game_scene_guyFrameData[gi].push_back(
                        GuyFrameData{
                            getFrameNumber(frame),
                            findCurrentGuy(frame->getView().getGuyInformation(), gi)
                        }
                    );
                    rgs_frameGuyData.push_back(gi);
                }
            }
        }
    }

    std::vector<std::vector<GuyFrameData>> guyFrameData;
    guyFrameData.reserve(run_game_scene_guyFrameData.size());
    for (std::size_t i{}, end{ std::size(run_game_scene_guyFrameData) }; i != end; ++i) {
        guyFrameData.push_back(run_game_scene_guyFrameData[i]);
        //guyFrameData.push_back(std::vector<GuyFrameData>());
        //guyFrameData[i].reserve(run_game_scene_guyFrameData[i].size());
        //for (std::size_t j{}, end{ std::size(run_game_scene_guyFrameData[i]) }; j != end; ++j) {
        //    guyFrameData[i].push_back(
        //        GuyFrameData{
        //            run_game_scene_guyFrameData[i][j].frameNumber,
        //            run_game_scene_guyFrameData[i][j].guyOutputInfo
        //        }
        //    );
        //}
    }

    // Old reliable code
    //auto const &guyFrames{timeEngine.getGuyFrames()};
    //auto const actualGuyFrames{ boost::make_iterator_range(guyFrames.begin(), std::prev(guyFrames.end())) };
    //
    //std::vector<std::vector<GuyFrameData>> guyFrameData;
    //guyFrameData.reserve(actualGuyFrames.size());
    //for (std::size_t i{}, end{ std::size(actualGuyFrames) }; i != end; ++i) {
    //    guyFrameData.push_back({});
    //    for (hg::Frame *frame : actualGuyFrames[i]) {
    //        guyFrameData[i].push_back(
    //            GuyFrameData{
    //                getFrameNumber(frame),
    //                *boost::find_if(frame->getView().getGuyInformation(), [i](auto const& guyInfo) {return guyInfo.getIndex() == i; })
    //            }
    //        );
    //    }
    //}

    return UIFrameState{
            drawnFrame,
            drawnTimeDirection,
            guyIndex,
            shouldDrawGuyPositionColours,
            shouldDrawInventory,
            pickups,
			input,
			paused,
            relativeGuyIndex,
            waveInfo,
            runningFromReplay,
            guyFrameUpdated,

            getGlitzForDirection(timeEngine.getFrame(drawnFrame)->getView(), drawnTimeDirection),
            timeEngine.getWall(),
            timeEngine.getPostOverwriteInput(),
            static_cast<std::size_t>(timeEngine.getTimelineLength()),
            std::move(guyFrameData)
    };
}

void saveReplayLog(std::ostream &toAppendTo, hg::InputList const &toAppend)
{
    toAppendTo << toAppend << " " << std::flush;
}


void generateReplay()
{
    std::ifstream replayLogIn("replayLogIn");
    if (replayLogIn.is_open()) {
        std::vector<hg::InputList> replay;
        replay.assign(std::istream_iterator<hg::InputList>(replayLogIn), std::istream_iterator<hg::InputList>());
        saveReplay("replay", replay);
    }
}
}
