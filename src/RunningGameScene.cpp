#include "RunningGameScene.h"
#include "Scene.h"
#include "Hg_Input.h"
#include "Inertia.h"
#include "LoadedLevel.h"
#include "ReplayIO.h"
#include "TimeEngine.h"
#include "GlobalConst.h"
#include <tbb/task_group.h>
#include <boost/thread/future.hpp>
#include <cassert>
#include <fstream>
#include "ObjectPtrList.h"
#include "ObjectListTypes.h"
#include "TimeEngine.h"
#include "PlayerVictoryException.h"
#include "Hg_Input.h"
#include "Level.h"
#include "Inertia.h"
#include "Frame.h"
#include "TestDriver.h"
#include "ReplayIO.h"
#include "LayeredCanvas.h"
#include "ResourceManager.h"
#include "RenderWindow.h"
#include "Maths.h"
#include "async.h"
#include <SFML/Graphics.hpp>

#include "AudioGlitzManager.h"
#include "ObjectPtrList.h"
#include "ObjectListTypes.h"
#include "TimeEngine.h"
#include "PlayerVictoryException.h"
#include "Hg_Input.h"
#include "Level.h"
#include "Inertia.h"
#include "Frame.h"
#include "TestDriver.h"
#include "ReplayIO.h"
#include "LayeredCanvas.h"
#include "ResourceManager.h"
#include "RenderWindow.h"
#include <SFML/Graphics.hpp>

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

#include "DirectLuaTriggerSystem.h"
#include "LevelLoader.h"
#include "move_function.h"
#include "unique_ptr.h"
#include "sfRenderTargetCanvas.h"
#include "sfColour.h"
#include "Maths.h"

#include "Scene.h"
#include "InitialScene.h"

#include "InitialScene.h"
#include "RenderWindow.h"
#include "LoadingLevelScene.h"
#include "LoadedLevel.h"
#include "PostLevelScene.h"
#include "move_function.h"
#include <functional>

#include "LoadingLevelScene.h"
#include <tbb/task_group.h>
#include <boost/thread/future.hpp>
#include "LoadedLevel.h"
#include "variant.h"
#include "GameDisplayHelpers.h"

namespace hg {
void runStep(
    hg::TimeEngine &timeEngine,
    hg::RenderWindow &app,
    hg::VulkanEngine &eng,
    AudioPlayingState &audioPlayingState,
    AudioGlitzManager &audioGlitzManager,
    std::size_t relativeGuyIndex,
    hg::Inertia &inertia,
    hg::TimeEngine::RunResult const &waveInfo,
    hg::LevelResources const &resources,
    sf::Image const &wallImage,
    sf::Image const &positionColoursImage,
    std::chrono::steady_clock::time_point &frameStartTime);


void saveReplayLog(std::ostream &toAppendTo, hg::InputList const &toAppend);
void generateReplay();
struct AwaitingInputState {
    
};

struct RunningLevelState {
    
};

ActivePanel const getActivePanel(hg::RenderWindow const &window)
{
    ActivePanel mousePanel = ActivePanel::NONE;
    // Todo, possibly include xFill and yFill.
    if (window.getInputState().getMousePosition().x > window.getSize().x*(hg::UI_DIVIDE_X))
    {
        int mouseY = window.getInputState().getMousePosition().y;
        if (mouseY <= window.getSize().y*(hg::UI_DIVIDE_Y))
        {
            mousePanel = ActivePanel::WORLD;
        }
        else if (mouseY <= (window.getSize().y*((hg::UI_DIVIDE_Y) + (hg::G_TIME_Y + hg::G_TIME_HEIGHT)*(1. - hg::UI_DIVIDE_Y))))
        {
            if (mouseY >= (window.getSize().y*((hg::UI_DIVIDE_Y) + hg::G_TIME_Y*(1. - hg::UI_DIVIDE_Y))))
            {
                mousePanel = ActivePanel::GLOBAL_TIME;
            }
        }
        else if (mouseY <= (window.getSize().y*((hg::UI_DIVIDE_Y) + (hg::P_TIME_Y + hg::P_TIME_HEIGHT)*(1. - hg::UI_DIVIDE_Y))))
        {
            if (mouseY >= (window.getSize().y*((hg::UI_DIVIDE_Y) + hg::P_TIME_Y*(1. - hg::UI_DIVIDE_Y))))
            {
                mousePanel = ActivePanel::PERSONAL_TIME;
            }
        }
    }
    return mousePanel;
}

variant<
    GameAborted_tag,
    GameWon_tag,
    ReloadLevel_tag,
    move_function<std::vector<hg::InputList>()>
>
run_game_scene(hg::RenderWindow &window, VulkanEngine &eng, LoadedLevel &&loadedLevel, std::vector<hg::InputList> const& replay)
{
    std::vector<InputList> receivedInputs;

    auto frameStartTime = std::chrono::steady_clock().now();
    TimeEngine const initialTimeEngine(loadedLevel.timeEngine);

    auto audioPlayingState = AudioPlayingState(loadedLevel.resources.sounds);
    auto audioGlitzManager = AudioGlitzManager();

    assert(loadedLevel.bakedWall);
    assert(loadedLevel.bakedPositionColours);

    hg::TimeEngine &timeEngine = loadedLevel.timeEngine;
    hg::LevelResources const &levelResources = loadedLevel.resources;
    sf::Image const &wallImage = *loadedLevel.bakedWall;
    sf::Image const &positionColoursImage = *loadedLevel.bakedPositionColours;

    enum class RunState { AWAITING_INPUT, RUNNING_LEVEL, PAUSED };
    RunState state(RunState::AWAITING_INPUT);

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

    auto interrupter = hg::make_unique<hg::OperationInterrupter>();
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
        switch (state) {
        case RunState::AWAITING_INPUT:
        {
            hg::InputList inputList;
            if (currentReplayIt != currentReplayEnd) {
                inputList = *currentReplayIt;
                ++currentReplayIt;
                runningFromReplay = true;
            }
            else {
                hg::Wall const &wall(timeEngine.getWall());
                double xScale = (window.getSize().x*(1. - hg::UI_DIVIDE_X)) / wall.roomWidth();
                double yScale = (window.getSize().y*hg::UI_DIVIDE_Y) / wall.roomHeight();
                double scalingFactor(std::min(xScale, yScale));
                double xFill = scalingFactor / xScale;
                double yFill = scalingFactor / yScale;
                int mouseOffX = static_cast<int>(window.getSize().x*(hg::UI_DIVIDE_X + (1. - xFill)*(1. - hg::UI_DIVIDE_X) / 2.));
                int mouseOffY = static_cast<int>(window.getSize().y*((1. - yFill)*hg::UI_DIVIDE_Y / 2.));
                int timelineOffset = static_cast<int>(window.getSize().x*(hg::UI_DIVIDE_X + hg::TIMELINE_PAD_X));
                int timelineWidth = static_cast<int>(window.getSize().x*((1.f - hg::UI_DIVIDE_X) - 2.f*hg::TIMELINE_PAD_X));
                int personalTimelineWidth = (timeEngine.getReplayData().size() > 0) ? std::min(timelineWidth, static_cast<int>(timelineWidth*timeEngine.getReplayData().size() / timeEngine.getTimelineLength())) : timelineWidth;
                
                ActivePanel mousePanel = getActivePanel(window);

                input.updateState(window.getInputState(), mousePanel,
                    timelineOffset, timelineWidth, personalTimelineWidth,
                    timeEngine.getReplayData().size(),
                    mouseOffX, mouseOffY, 1. / scalingFactor);
                inputList = input.AsInputList();
                runningFromReplay = false;
            }
            relativeGuyIndex = inputList.getRelativeGuyIndex();
            saveReplayLog(replayLogOut, inputList);
            receivedInputs.push_back(inputList);
            interrupter = make_unique<hg::OperationInterrupter>();
            
            futureRunResult =
                async(
                    [inputList, &timeEngine, &interrupter] {
                return timeEngine.runToNextPlayerFrame(std::move(inputList), *interrupter);});
            state = RunState::RUNNING_LEVEL;
            break;
        }

        case RunState::RUNNING_LEVEL:
        {
            sf::Event event;
            while (window.pollEvent(event))
            {
                //States + transitions:
                //Not really a state machine!
                //Playing game -> new game + playing game               Keybinding: R
                //playing game -> new game + playing replay             Keybinding: L

                //playing replay -> new game + playing game             Keybinding: R
                //playing replay -> new game + playing replay           Keybinding: L
                //playing replay -> playing game                        Keybinding: C or <get to end of replay>
                switch (event.type) {
                case sf::Event::Closed:
                    window.close();
                    throw WindowClosed_exception{};
                case sf::Event::KeyPressed:
                    switch (event.key.code) {
                    case sf::Keyboard::Escape:
                        return GameAborted_tag{};
                    //Restart
                    case sf::Keyboard::R:
                        return ReloadLevel_tag{};
                    //Load replay
                    case sf::Keyboard::L:
                        return move_function<std::vector<InputList>()>([] {return loadReplay("replay");});
                    //Interrupt replay and begin Playing
                    case sf::Keyboard::C:
                        currentReplayIt = replay.end();
                        currentReplayEnd = replay.end();
                        break;
                    //Save replay
                    case sf::Keyboard::K:
                        saveReplay("replay", receivedInputs);
                        break;
                    //Generate a replay from replayLogIn
                    case sf::Keyboard::G:
                        generateReplay();
                        break;
                    case sf::Keyboard::P:
                        state = RunState::PAUSED;
                        goto continuemainloop;

                    default:
                        break;
                    }
                    break;
                default:
                    break;
                }
            }
            if (futureRunResult.wait_for(boost::chrono::duration<double>(1.f / (hg::FRAMERATE))) == boost::future_status::ready) {
                if (window.getInputState().isKeyPressed(sf::Keyboard::Period)) {
                    inertia.save(mousePosToFrameID(window, timeEngine), TimeDirection::FORWARDS);
                }
                if (window.getInputState().isKeyPressed(sf::Keyboard::Comma)) {
                    inertia.save(mousePosToFrameID(window, timeEngine), TimeDirection::REVERSE);
                }
                if (window.getInputState().isKeyPressed(sf::Keyboard::Slash)) {
                    inertia.reset();
                }
                try {
                    assert(futureRunResult.get_state() != boost::future_state::uninitialized);
                    runStep(timeEngine, window, eng, audioPlayingState, audioGlitzManager, relativeGuyIndex, inertia, futureRunResult.get(), levelResources, wallImage, positionColoursImage, frameStartTime);
                    interrupter.reset();
                }
                catch (hg::PlayerVictoryException const &) {
                    run_post_level_scene(window, eng, initialTimeEngine, loadedLevel);
                    //TODO -- Check run_post_level_scene return values (once it gets return values)
                    return GameWon_tag{};
                }
                if (runningFromReplay) {
                    //TODO: also write some sort of replay progress display here
                    //currentReplayIt-replay.begin() << "/" << currentReplayEnd-replay.begin()


                    sf::Text replayGlyph;
                    replayGlyph.setFont(*hg::defaultFont);
                    replayGlyph.setString("R");
                    replayGlyph.setFillColor(sf::Color(255, 25, 50));
                    replayGlyph.setOutlineColor(sf::Color(255, 25, 50));
                    replayGlyph.setPosition(580, 32);
                    replayGlyph.setCharacterSize(32);
                    window.draw(replayGlyph);
                }
                if (window.getInputState().isKeyPressed(sf::Keyboard::F)) {
                    window.setFramerateLimit(0);
                    window.setVerticalSyncEnabled(false);
                }
                else {
                    window.setFramerateLimit(hg::FRAMERATE);
                    window.setVerticalSyncEnabled(true);
                }
                window.display();
                state = RunState::AWAITING_INPUT;
            }
            break;
        }
        case RunState::PAUSED:
        {
            {
                sf::Event event;
                while (window.waitEvent(event))
                {
                    switch (event.type) {
                    case sf::Event::Closed:
                        window.close();
                        throw WindowClosed_exception{};
                    case sf::Event::KeyPressed:
                        switch (event.key.code) {
                        case sf::Keyboard::P:
                            state = RunState::RUNNING_LEVEL;
                            goto continuemainloop;
                        default: break;
                        }
                        break;
                    default: break;
                    }
                }
            }
            sf::sleep(sf::seconds(.1f));
        }
        }
    continuemainloop:;
    }
    assert(false && "should be unreachable");
}


hg::mt::std::vector<hg::Glitz> const &getGlitzForDirection(
    hg::FrameView const &view, hg::TimeDirection timeDirection)
{
    return timeDirection == TimeDirection::FORWARDS ? view.getForwardsGlitz() : view.getReverseGlitz();
}


void runStep(
    hg::TimeEngine &timeEngine,
    hg::RenderWindow &app,
    hg::VulkanEngine &eng,
    AudioPlayingState &audioPlayingState,
    AudioGlitzManager &audioGlitzManager,
    std::size_t relativeGuyIndex,
    hg::Inertia &inertia,
    hg::TimeEngine::RunResult const &waveInfo,
    hg::LevelResources const &resources,
    sf::Image const &wallImage,
    sf::Image const &positionColoursImage,
    std::chrono::steady_clock::time_point &frameStartTime)
{
    hg::FrameID drawnFrame;
    hg::TimeDirection drawnTimeDirection{hg::TimeDirection::INVALID};
    hg::mt::std::map<hg::Ability, int> pickups;
    hg::Ability abilityCursor{hg::Ability::NO_ABILITY};
    bool shouldDrawInventory{false};
    bool const shouldDrawGuyPositionColours{app.getInputState().isKeyPressed(sf::Keyboard::LShift)};
    std::size_t guyIndex = timeEngine.getGuyFrames().size() - 2 - relativeGuyIndex;

    ActivePanel mousePanel = getActivePanel(app);
    if (app.getInputState().isKeyPressed(sf::Keyboard::LControl) && mousePanel != ActivePanel::PERSONAL_TIME) {
        drawnFrame = mousePosToFrameID(app, timeEngine);
        drawnTimeDirection = TimeDirection::FORWARDS;
    }
    else {
        if (app.getInputState().isKeyPressed(sf::Keyboard::LControl) && mousePanel == ActivePanel::PERSONAL_TIME) {
            guyIndex = mousePosToGuyIndex(app, timeEngine);
            relativeGuyIndex = timeEngine.getGuyFrames().size() - 2 - guyIndex;
        }

        if (hg::Frame *const guyFrame{timeEngine.getGuyFrames()[guyIndex]}; !isNullFrame(guyFrame)) {
            hg::GuyOutputInfo const &currentGuy(findCurrentGuy(guyFrame->getView().getGuyInformation(), guyIndex));

            drawnTimeDirection = currentGuy.getTimeDirection();
            drawnFrame = hg::FrameID(guyFrame);
            inertia.save(drawnFrame, drawnTimeDirection);

            shouldDrawInventory = true;
            pickups = currentGuy.getPickups();
            abilityCursor = timeEngine.getPostOverwriteInput()[guyIndex].getAbilityCursor();
        }
        else {
            inertia.run();
            if (hg::FrameID const inertialFrame(inertia.getFrame()); inertialFrame.isValidFrame()) {
                drawnFrame = inertialFrame;
                drawnTimeDirection = inertia.getTimeDirection();
            }
            else {
                drawnFrame = mousePosToFrameID(app, timeEngine);
                drawnTimeDirection = TimeDirection::FORWARDS;
            }
        }
    }

    auto const &glitz{getGlitzForDirection(timeEngine.getFrame(drawnFrame)->getView(), drawnTimeDirection)};

    //TODO: Possbily sync with Graphics?, via
    //Vulkan Timestamp Queries:
    //https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#queries-timestamps

    //TODO: Support slowing down/stopping sound if game is playing slowly, or speeding up sound
    //      if game is going fast; Support proper audio rendering of timeline scrubbing.
    //      Somehow support proper sound in Pause Time.
    //      etc.
    PlayAudioGlitz(
        glitz,
        audioPlayingState,
        audioGlitzManager,
        static_cast<int>(guyIndex)
    );
    //TODO: Play UI Sounds

    app.clear(sf::Color(255, 255, 255));

    DrawVisualGlitzAndWall(
        app.getRenderTarget(),
        eng,
        glitz,
        timeEngine.getWall(),
        resources,
        wallImage,
        positionColoursImage,
        static_cast<int>(guyIndex),
        shouldDrawGuyPositionColours);

    if (shouldDrawInventory) {
        drawInventory(
            app.getRenderTarget(),
            pickups,
            abilityCursor);
    }

    DrawTimeline(
        app.getRenderTarget(),
        timeEngine,
        waveInfo.updatedFrames,
        drawnFrame,
        timeEngine.getReplayData()[timeEngine.getReplayData().size() - 1].getGuyInput().getTimeCursor(),
        timeEngine.getTimelineLength());

    DrawPersonalTimeline(
        app.getRenderTarget(),
        timeEngine,
        relativeGuyIndex,
        timeEngine.getGuyFrames(),
        timeEngine.getPostOverwriteInput(),
        static_cast<std::size_t>(timeEngine.getTimelineLength()));

    DrawInterfaceBorder(app.getRenderTarget());

    {
        std::stringstream currentPlayerIndex;
        currentPlayerIndex << "Index: " << timeEngine.getReplayData().size() - 1;
        sf::Text currentPlayerGlyph;
        currentPlayerGlyph.setFont(*hg::defaultFont);
        currentPlayerGlyph.setString(currentPlayerIndex.str());
        currentPlayerGlyph.setPosition(90, static_cast<float>(hg::WINDOW_DEFAULT_Y) - 55);
        currentPlayerGlyph.setCharacterSize(16);
        currentPlayerGlyph.setFillColor(uiTextColor);
        currentPlayerGlyph.setOutlineColor(uiTextColor);
        app.draw(currentPlayerGlyph);
    }
    {
        std::stringstream currentPlayerIndex;
        currentPlayerIndex << "Control: " << timeEngine.getReplayData().size() - 1 - relativeGuyIndex;
        sf::Text currentPlayerGlyph;
        currentPlayerGlyph.setFont(*hg::defaultFont);
        currentPlayerGlyph.setString(currentPlayerIndex.str());
        currentPlayerGlyph.setPosition(90, static_cast<float>(hg::WINDOW_DEFAULT_Y) - 35);
        currentPlayerGlyph.setCharacterSize(16);
        currentPlayerGlyph.setFillColor(uiTextColor);
        currentPlayerGlyph.setOutlineColor(uiTextColor);
        app.draw(currentPlayerGlyph);
    }
    {
        std::stringstream frameNumberString;
        frameNumberString << "Frame: " << drawnFrame.getFrameNumber();
        sf::Text frameNumberGlyph;
        frameNumberGlyph.setFont(*hg::defaultFont);
        frameNumberGlyph.setString(frameNumberString.str());
        frameNumberGlyph.setPosition(90, static_cast<float>(hg::WINDOW_DEFAULT_Y*hg::UI_DIVIDE_Y) + 60);
        frameNumberGlyph.setCharacterSize(16);
        frameNumberGlyph.setFillColor(uiTextColor);
        frameNumberGlyph.setOutlineColor(uiTextColor);
        app.draw(frameNumberGlyph);
    }
    {
        std::stringstream timeString;
        timeString << "Time: " << (drawnFrame.getFrameNumber()*10 / hg::FRAMERATE)/10. << "s";
        sf::Text frameNumberGlyph;
        frameNumberGlyph.setFont(*hg::defaultFont);
        frameNumberGlyph.setString(timeString.str());
        frameNumberGlyph.setPosition(90, static_cast<float>(hg::WINDOW_DEFAULT_Y*hg::UI_DIVIDE_Y) + 20);
        frameNumberGlyph.setCharacterSize(16);
        frameNumberGlyph.setFillColor(uiTextColor);
        frameNumberGlyph.setOutlineColor(uiTextColor);
        app.draw(frameNumberGlyph);
    }
    {
        std::vector<int> framesExecutedList;
        framesExecutedList.reserve(boost::size(waveInfo.updatedFrames));
        for (
            hg::FrameUpdateSet const &updateSet :
            waveInfo.updatedFrames)
        {
            framesExecutedList.push_back(static_cast<int>(boost::size(updateSet)));
        }
        std::stringstream numberOfFramesExecutedString;
        if (!boost::empty(framesExecutedList)) {
            numberOfFramesExecutedString << *boost::begin(framesExecutedList);
            for (
                int num:
                framesExecutedList
                | boost::adaptors::sliced(1, boost::size(framesExecutedList)))
            {
                numberOfFramesExecutedString << ":" << num;
            }
        }
        sf::Text numberOfFramesExecutedGlyph;
        numberOfFramesExecutedGlyph.setFont(*hg::defaultFont);
        numberOfFramesExecutedGlyph.setString(numberOfFramesExecutedString.str());
        numberOfFramesExecutedGlyph.setPosition(50, 330);
        numberOfFramesExecutedGlyph.setCharacterSize(12);
        numberOfFramesExecutedGlyph.setFillColor(uiTextColor);
        numberOfFramesExecutedGlyph.setOutlineColor(uiTextColor);
        app.draw(numberOfFramesExecutedGlyph);
    }
    {
        auto newFrameStartTime = std::chrono::steady_clock().now();
        std::stringstream fpsstring;
        auto const fps = (1. / std::chrono::duration<double>(newFrameStartTime - frameStartTime).count());
        fpsstring << (fps > 5 ? std::round(fps) : fps);
        frameStartTime = newFrameStartTime;
        sf::Text fpsglyph;
        fpsglyph.setFont(*hg::defaultFont);
        fpsglyph.setString(fpsstring.str());
        fpsglyph.setCharacterSize(12);
        fpsglyph.setFillColor(uiTextColor);
        fpsglyph.setOutlineColor(uiTextColor);
        fpsglyph.setPosition(50, 300);
        app.draw(fpsglyph);
        //std::cout << "fps: " << fps << "\n";
    }
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
