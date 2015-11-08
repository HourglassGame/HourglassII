#include "RunningGameScene.h"
#include "Scene.h"
#include "Hg_Input.h"
#include "Inertia.h"
#include "LoadedLevel.h"
#include "ReplayIO.h"
#include "TimeEngine.h"
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
#include "enqueue_task.h"
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
#include <iostream>
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
    AudioPlayingState &audioPlayingState,
    AudioGlitzManager &audioGlitzManager,
    hg::Inertia &inertia,
    hg::TimeEngine::RunResult const &waveInfo,
    hg::LevelResources const &resources,
    sf::Image const &wallImage,
    std::chrono::steady_clock::time_point &frameStartTime);


void saveReplayLog(std::ostream &toAppendTo, hg::InputList const &toAppend);
void generateReplay();
struct AwaitingInputState {
    
};

struct RunningLevelState {
    
};

variant<
    GameAborted_tag,
    GameWon_tag,
    ReloadLevel_tag,
    move_function<std::vector<hg::InputList>()>
>
run_game_scene(hg::RenderWindow &window, LoadedLevel &&loadedLevel, std::vector<hg::InputList> const& replay)
{
    std::vector<InputList> receivedInputs;

    auto frameStartTime = std::chrono::steady_clock().now();
    LoadedLevel const initialLevel(loadedLevel);

    auto audioPlayingState = AudioPlayingState(loadedLevel.resources.sounds);
    auto audioGlitzManager = AudioGlitzManager();

    hg::TimeEngine &timeEngine = loadedLevel.timeEngine;
    hg::LevelResources const &levelResources = loadedLevel.resources;
    sf::Image const &wallImage = loadedLevel.bakedWall;

    enum RunState { AWAITING_INPUT, RUNNING_LEVEL, PAUSED };
    RunState state(AWAITING_INPUT);

    hg::Input input;
    input.setTimelineLength(timeEngine.getTimelineLength());
    hg::Inertia inertia;

    std::vector<hg::InputList>::const_iterator currentReplayIt(replay.begin());
    std::vector<hg::InputList>::const_iterator currentReplayEnd(replay.end());
    //Saves a replay continuously, but in a less nice format.
    //Gets rewritten whenever the level is reset.
    //Useful for tracking down crashes.
    std::ofstream replayLogOut("replayLogOut");
    tbb::task_group task_group;
    struct task_group_waiter {
        tbb::task_group &tg;
        ~task_group_waiter() {
            tg.wait();
        }
    } task_group_waiter_obj{task_group};
    
    auto interrupter = hg::make_unique<hg::OperationInterrupter>();
    boost::future<hg::TimeEngine::RunResult> futureRunResult;
    
    struct TimeEngineCleanupEnforcer {
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
        case AWAITING_INPUT:
        {
            hg::InputList inputList;
            if (currentReplayIt != currentReplayEnd) {
                inputList = *currentReplayIt;
                ++currentReplayIt;
                runningFromReplay = true;
            }
            else {
                hg::Wall const &wall(timeEngine.getWall());
                double scalingFactor(std::max(wall.roomWidth()*1. / window.getSize().x, wall.roomHeight()*1. / window.getSize().y));
                input.updateState(window.getInputState(), window.getSize().x, scalingFactor);
                inputList = input.AsInputList();
                runningFromReplay = false;
            }
            saveReplayLog(replayLogOut, inputList);
            receivedInputs.push_back(inputList);
            interrupter = make_unique<hg::OperationInterrupter>();
            
            futureRunResult =
                enqueue_task(
                    task_group,
                    [inputList, &timeEngine, &interrupter] {
                return timeEngine.runToNextPlayerFrame(std::move(inputList), *interrupter);});
            state = RUNNING_LEVEL;
            break;
        }

        case RUNNING_LEVEL:
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
                        std::cout << "Returning GameAborted\n";
                        return GameAborted_tag{};
                        //Restart
                    case sf::Keyboard::R:
                        interrupter->interrupt();
                        futureRunResult.wait();
                        receivedInputs.clear();
                        timeEngine = initialLevel.timeEngine;
                        loadedLevel.resources = initialLevel.resources;
                        loadedLevel.bakedWall = initialLevel.bakedWall;
                        audioPlayingState = AudioPlayingState(initialLevel.resources.sounds);
                        audioGlitzManager = AudioGlitzManager();
                        state = AWAITING_INPUT;
                        goto continuemainloop;
                        //return ReloadLevel_tag{};
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
                        state = PAUSED;
                        goto continuemainloop;

                    default:
                        break;
                    }
                    break;
                default:
                    break;
                }
            }
            if (futureRunResult.wait_for(boost::chrono::duration<double>(1.f / (60.f))) == boost::future_status::ready) {
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
                    runStep(timeEngine, window, audioPlayingState, audioGlitzManager, inertia, futureRunResult.get(), levelResources, wallImage, frameStartTime);
                    interrupter.reset();
                }
                catch (hg::PlayerVictoryException const &) {
                    run_post_level_scene(window, initialLevel, loadedLevel);
                    //TODO -- Check run_post_level_scene return values (once it gets return values)
                    return GameWon_tag{};
                }
                if (runningFromReplay) {
                    sf::Text replayGlyph;
                    replayGlyph.setFont(*hg::defaultFont);
                    replayGlyph.setString("R");
                    replayGlyph.setColor(sf::Color(255, 25, 50));
                    replayGlyph.setPosition(580, 32);
                    replayGlyph.setCharacterSize(32);
                    window.draw(replayGlyph);
                }
                if (window.getInputState().isKeyPressed(sf::Keyboard::F)) {
                    window.setFramerateLimit(0);
                    window.setVerticalSyncEnabled(false);
                }
                else {
                    window.setFramerateLimit(60);
                    window.setVerticalSyncEnabled(true);
                }
                window.display();
                state = AWAITING_INPUT;
            }
            break;
        }
        case PAUSED:
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
                            state = RUNNING_LEVEL;
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
    AudioPlayingState &audioPlayingState,
    AudioGlitzManager &audioGlitzManager,
    hg::Inertia &inertia,
    hg::TimeEngine::RunResult const &waveInfo,
    hg::LevelResources const &resources,
    sf::Image const &wallImage,
    std::chrono::steady_clock::time_point &frameStartTime)
{
    std::vector<int> framesExecutedList;
    hg::FrameID drawnFrame;

    framesExecutedList.reserve(boost::distance(waveInfo.updatedFrames));
    for (
        hg::FrameUpdateSet const &updateSet:
        waveInfo.updatedFrames)
    {
        framesExecutedList.push_back(static_cast<int>(boost::distance(updateSet)));
    }

    if (app.getInputState().isKeyPressed(sf::Keyboard::LControl)) {
        drawnFrame =
              hg::FrameID(
                abs(
                  static_cast<int>(
                    hg::flooredModulo(static_cast<long>((sf::Mouse::getPosition(app.getWindow()).x
                     * static_cast<long>(timeEngine.getTimelineLength())
                     / app.getSize().x))
                    , static_cast<long>(timeEngine.getTimelineLength())))),
                hg::UniverseID(timeEngine.getTimelineLength()));
        hg::Frame const *frame(timeEngine.getFrame(drawnFrame));
        DrawGlitzAndWall(app,
             getGlitzForDirection(frame->getView(), TimeDirection::FORWARDS),
             timeEngine.getWall(),
             resources,
             audioPlayingState,
             audioGlitzManager,
             wallImage);
    }
    else if (waveInfo.currentPlayerFrame) {
        hg::FrameView const &view(waveInfo.currentPlayerFrame->getView());
        hg::GuyOutputInfo const &currentGuy(findCurrentGuy(view.getGuyInformation()));
        hg::TimeDirection currentGuyDirection(currentGuy.getTimeDirection());
        inertia.save(hg::FrameID(waveInfo.currentPlayerFrame), currentGuyDirection);
        drawnFrame = hg::FrameID(waveInfo.currentPlayerFrame);
        DrawGlitzAndWall(
            app,
            getGlitzForDirection(view, currentGuyDirection),
            timeEngine.getWall(),
            resources,
            audioPlayingState,
            audioGlitzManager,
            wallImage);
        
        drawInventory(
            app,
            findCurrentGuy(view.getGuyInformation()).getPickups(),
            timeEngine.getReplayData().back().getAbilityCursor());
    }
    else {
        inertia.run();
        hg::FrameID const inertialFrame(inertia.getFrame());
        if (inertialFrame.isValidFrame()) {
            drawnFrame = inertialFrame;
            hg::Frame const *frame(timeEngine.getFrame(inertialFrame));
            DrawGlitzAndWall(app,
                 getGlitzForDirection(frame->getView(), inertia.getTimeDirection()),
                 timeEngine.getWall(),
                 resources,
                 audioPlayingState,
                 audioGlitzManager,
                 wallImage);
        }
        else {
            drawnFrame =
              hg::FrameID(
                static_cast<int>(
                  hg::flooredModulo(
                    static_cast<long>(
                      sf::Mouse::getPosition(app.getWindow()).x *
                      static_cast<long>(timeEngine.getTimelineLength())/app.getSize().x),
                    static_cast<long>(timeEngine.getTimelineLength()))),
                hg::UniverseID(timeEngine.getTimelineLength()));
            
            hg::Frame const *frame(timeEngine.getFrame(drawnFrame));
            DrawGlitzAndWall(
                app,
                getGlitzForDirection(frame->getView(), TimeDirection::FORWARDS),
                timeEngine.getWall(),
                resources,
                audioPlayingState,
                audioGlitzManager,
                wallImage);
        }
    }
    
    DrawTimeline(
        app.getRenderTarget(),
        timeEngine,
        waveInfo.updatedFrames,
        drawnFrame,
        timeEngine.getReplayData().back().getTimeCursor(),
        timeEngine.getTimelineLength());
    
    {
        std::stringstream currentPlayerIndex;
        currentPlayerIndex << "Index: " << timeEngine.getReplayData().size() - 1;
        sf::Text currentPlayerGlyph;
        currentPlayerGlyph.setFont(*hg::defaultFont);
        currentPlayerGlyph.setString(currentPlayerIndex.str());
        currentPlayerGlyph.setPosition(580, 433);
        currentPlayerGlyph.setCharacterSize(10.f);
        currentPlayerGlyph.setColor(uiTextColor);
        app.draw(currentPlayerGlyph);
    }
    {
        std::stringstream frameNumberString;
        frameNumberString << "Frame: " << drawnFrame.getFrameNumber();
        sf::Text frameNumberGlyph;
        frameNumberGlyph.setFont(*hg::defaultFont);
        frameNumberGlyph.setString(frameNumberString.str());
        frameNumberGlyph.setPosition(580, 445);
        frameNumberGlyph.setCharacterSize(8.f);
        frameNumberGlyph.setColor(uiTextColor);
        app.draw(frameNumberGlyph);
    }
    {
        std::stringstream numberOfFramesExecutedString;
        if (!boost::empty(framesExecutedList)) {
            numberOfFramesExecutedString << *boost::begin(framesExecutedList);
            for (
                int num:
                framesExecutedList
                | boost::adaptors::sliced(1, boost::distance(framesExecutedList)))
            {
                numberOfFramesExecutedString << ":" << num;
            }
        }
        sf::Text numberOfFramesExecutedGlyph;
        numberOfFramesExecutedGlyph.setFont(*hg::defaultFont);
        numberOfFramesExecutedGlyph.setString(numberOfFramesExecutedString.str());
        numberOfFramesExecutedGlyph.setPosition(580, 455);
        numberOfFramesExecutedGlyph.setCharacterSize(8.f);
        numberOfFramesExecutedGlyph.setColor(uiTextColor);
        app.draw(numberOfFramesExecutedGlyph);
    }
    {
        auto newFrameStartTime = std::chrono::steady_clock().now();
        std::stringstream fpsstring;
        fpsstring << (1./std::chrono::duration<double>(newFrameStartTime-frameStartTime).count());
        frameStartTime = newFrameStartTime;
        sf::Text fpsglyph;
        fpsglyph.setFont(*hg::defaultFont);
        fpsglyph.setString(fpsstring.str());
        fpsglyph.setPosition(600, 465);
        fpsglyph.setCharacterSize(8.f);
        fpsglyph.setColor(uiTextColor);
        app.draw(fpsglyph);
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
