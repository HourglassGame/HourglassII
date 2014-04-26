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
#include "Foreach.h"
#include "ReplayIO.h"
#include "LayeredCanvas.h"
#include "ResourceManager.h"
#include "RenderWindow.h"
#include "Maths.h"
#include <SFML/Graphics.hpp>

#include "ObjectPtrList.h"
#include "ObjectListTypes.h"
#include "TimeEngine.h"
#include "PlayerVictoryException.h"
#include "Hg_Input.h"
#include "Level.h"
#include "Inertia.h"
#include "Frame.h"
#include "TestDriver.h"
#include "Foreach.h"
#include "ReplayIO.h"
#include "LayeredCanvas.h"
#include "ResourceManager.h"
#include "RenderWindow.h"
#include <SFML/Graphics.hpp>

#include <boost/multi_array.hpp>
#include <boost/assign.hpp>
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
#include "ConcurrentQueue.h"
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
    hg::Inertia &inertia,
    hg::TimeEngine::RunResult const &waveInfo,
    hg::LevelResources const &resources,
    sf::Image const &wallImage);


void saveReplayLog(std::ostream &toAppendTo, hg::InputList const &toAppend);
void generateReplay();

struct RunToNextPlayerFrame
{
    RunToNextPlayerFrame(hg::TimeEngine &timeEngine, hg::InputList const &inputList, hg::OperationInterrupter &interrupter) :
        timeEngine_(&timeEngine),
        inputList_(inputList),
        interrupter_(&interrupter)
    {}
    typedef hg::TimeEngine::RunResult result_type;
    hg::TimeEngine::RunResult operator()()
    {
        return timeEngine_->runToNextPlayerFrame(boost::move(inputList_), *interrupter_);
    }
private:
    hg::TimeEngine *timeEngine_;
    hg::InputList inputList_;
    hg::OperationInterrupter *interrupter_;
};


struct CreateTimeEngine {
    //boost::result_of support
    typedef hg::TimeEngine result_type;

    CreateTimeEngine(std::string const &filename, hg::OperationInterrupter &interrupter)
        : filename(filename), interrupter(&interrupter) {}

    hg::TimeEngine operator()() const {
        return hg::TimeEngine(hg::loadLevelFromFile(filename, *interrupter), *interrupter);
    }
private:
    std::string filename;
    hg::OperationInterrupter *interrupter;
};
    
template<typename F>
boost::future<typename boost::result_of<F()>::type> enqueue_task(tbb::task_group& queue, F f)
{
    auto task = std::make_shared<boost::packaged_task<typename boost::result_of<F()>::type()>>(f);
    auto copyable_task = [=]{return (*task)();};
    boost::future<typename boost::result_of<F()>::type> future(task->get_future());
    queue.run(copyable_task);
    return boost::move(future);
}

variant<
    GameAborted_tag,
    GameWon_tag,
    ReloadLevel_tag,
    move_function<std::vector<hg::InputList>()>
>
run_game_scene(hg::RenderWindow &window, LoadedLevel &&loadedLevel, std::vector<hg::InputList> const& replay)
{
    LoadedLevel const initialLevel(loadedLevel);

    tbb::task_group task_group;

    hg::unique_ptr<hg::OperationInterrupter> interrupter(new hg::OperationInterrupter());
    hg::TimeEngine &timeEngine = loadedLevel.timeEngine;
    hg::LevelResources &levelResources = loadedLevel.resources;
    sf::Image &wallImage = loadedLevel.bakedWall;

    enum {AWAITING_INPUT, RUNNING_LEVEL, PAUSED} state(AWAITING_INPUT);

    hg::Input input;
    input.setTimelineLength(timeEngine.getTimelineLength());
    hg::Inertia inertia;
    
    //std::vector<hg::InputList> replay;
    std::vector<hg::InputList>::const_iterator currentReplayIt(replay.begin());
    std::vector<hg::InputList>::const_iterator currentReplayEnd(replay.end());
    //Saves a replay continuously, but in a less nice format.
    //Gets rewritten whenever the level is reset.
    //Useful for tracking down crashes.
    std::ofstream replayLogOut("replayLogOut");
    boost::future<hg::TimeEngine::RunResult> futureRunResult;
    bool runningFromReplay(false);
    while (window.isOpen()) {
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
                    double scalingFactor(std::max(wall.roomWidth()*1./window.getSize().x, wall.roomHeight()*1./window.getSize().y));
					input.updateState(window.getInputState(), window.getSize().x, scalingFactor);
					inputList = input.AsInputList();
					runningFromReplay = false;
				}
				saveReplayLog(replayLogOut, inputList);
                interrupter = make_unique<hg::OperationInterrupter>();
                futureRunResult =
                    enqueue_task(
                        task_group,
                        RunToNextPlayerFrame(timeEngine, inputList, *interrupter));
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
                        interrupter->interrupt();
                        futureRunResult.wait();
						window.close();
						throw WindowClosed_exception{};
					case sf::Event::KeyPressed:
						switch(event.key.code) {
                        case sf::Keyboard::Escape:
                            interrupter->interrupt();
                            futureRunResult.wait();
                            return GameAborted_tag{};
						//Restart
						case sf::Keyboard::R:
                            //interrupter->interrupt();
                            futureRunResult.wait();
                            timeEngine = initialLevel.timeEngine;
                            levelResources = initialLevel.resources;
                            wallImage = initialLevel.bakedWall;
                            state = AWAITING_INPUT;
                            goto continuemainloop;
                            //return ReloadLevel_tag{};
                            /*
							currentReplayIt = replay.end();
							currentReplayEnd = replay.end();
							replayLogOut.close();
							replayLogOut.open("replayLogOut");
                            interrupter->interrupt();
							futureRunResult.wait();
                            interrupter = hg::unique_ptr<hg::OperationInterrupter>(new hg::OperationInterrupter());
						    futureTimeEngine =
						    	enqueue_task(
						    		task_group,
						    		CreateTimeEngine(levelPath, *interrupter));
							state = LOADING_LEVEL;
							goto continuemainloop;*/
						//Load replay
                        
						case sf::Keyboard::L:
                            return move_function<std::vector<InputList>()>([]{return loadReplay("replay");});
                        #if 0
							replay = hg::loadReplay("replay");
							currentReplayIt = replay.begin();
							currentReplayEnd = replay.end();
							replayLogOut.close();
							replayLogOut.open("replayLogOut");
                            interrupter->interrupt();
							futureRunResult.wait();
                            interrupter = hg::unique_ptr<hg::OperationInterrupter>(new hg::OperationInterrupter());
						    futureTimeEngine =
						    	enqueue_task(
						    		task_group,
						    		CreateTimeEngine(levelPath, *interrupter));
							state = LOADING_LEVEL;
							goto continuemainloop;
                        #endif
						//Interrupt replay and begin Playing
						case sf::Keyboard::C:
							currentReplayIt = replay.end();
							currentReplayEnd = replay.end();
                        break;
						//Save replay
						case sf::Keyboard::K:
							//TODO - possibly allow finer-grained access
							//to replay data, even when the step has not
							//yet completely finished. (ie allow access as soon as
							//the new input is pushed into the vector, rather than
							//waiting until the execution is totally finished as
							//we do now).
							futureRunResult.wait();
							saveReplay("replay", timeEngine.getReplayData());
                        break;
						//Generate a replay from replayLogIn
						case sf::Keyboard::G:
							generateReplay();
                        break;
                        case sf::Keyboard::P:
                            state = PAUSED;
                            goto continuemainloop;
                        break;
                        
						default:
							break;
						}
						break;
					default:
						break;
					}
				}
				if (futureRunResult.is_ready()) {
                    if (window.getInputState().isKeyPressed(sf::Keyboard::Period)) {
                        inertia.save(mousePosToFrameID(window, timeEngine), hg::FORWARDS);
                    }
                    if (window.getInputState().isKeyPressed(sf::Keyboard::Comma)) {
                        inertia.save(mousePosToFrameID(window, timeEngine), hg::REVERSE);
                    }
                    if (window.getInputState().isKeyPressed(sf::Keyboard::Slash)) {
                        inertia.reset();
                    }
					try {
						runStep(timeEngine, window, inertia, futureRunResult.get(), levelResources, wallImage);
                        interrupter.reset();
					}
					catch (hg::PlayerVictoryException const &) {
                        run_post_level_scene(window, initialLevel, loadedLevel);
                        //TODO -- Check run_post_level_scene return values
                        // e.g. WindowClosed, etc
						return GameWon_tag{};
					}
					if (runningFromReplay) {
						sf::Text replayGlyph;
                        replayGlyph.setFont(*hg::defaultFont);
                        replayGlyph.setString("R");
						replayGlyph.setColor(sf::Color(255,0,0));
						replayGlyph.setPosition(580, 32);
						replayGlyph.setCharacterSize(32.f);
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
                else {
                    sf::sleep(sf::seconds(1.f/(60.f*3.f)));
                }
				break;
			}
            case PAUSED:
            {
                {
                    sf::Event event;
                    while (window.pollEvent(event))
                    {
                        switch(event.type) {
                        case sf::Event::Closed:
                            interrupter->interrupt();
                            futureRunResult.wait();
                            window.close();
                            goto breakmainloop;
                        case sf::Event::KeyPressed:
						    switch(event.key.code) {
                            case sf::Keyboard::P:
                                state = RUNNING_LEVEL;
                                break;
                            default: break;
                            }
                        default: break;
                        }
                    }
                }
                sf::sleep(sf::seconds(.1f));
            }
    	}
    	continuemainloop:;
    }
    breakmainloop:;
    //timeEngineThread.interrupt();
    //timeEngineThread.join();

    //return EXIT_SUCCESS;
    throw WindowClosed_exception{};
}


hg::mt::std::vector<hg::Glitz>::type const &getGlitzForDirection(
    hg::FrameView const &view, hg::TimeDirection timeDirection)
{
    return timeDirection == hg::FORWARDS ? view.getForwardsGlitz() : view.getReverseGlitz();
}


void runStep(
    hg::TimeEngine &timeEngine,
    hg::RenderWindow &app,
    hg::Inertia &inertia,
    hg::TimeEngine::RunResult const &waveInfo,
    hg::LevelResources const &resources,
    sf::Image const &wallImage)
{
    std::vector<int> framesExecutedList;
    hg::FrameID drawnFrame;

    framesExecutedList.reserve(boost::distance(waveInfo.updatedFrames));
    foreach (
        hg::FrameUpdateSet const &updateSet,
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
             getGlitzForDirection(frame->getView(), hg::FORWARDS),
             timeEngine.getWall(),
             resources,
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
            wallImage);
        
        drawInventory(app, findCurrentGuy(view.getGuyInformation()).getPickups(), timeEngine.getReplayData().back().getAbilityCursor());
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
                 wallImage);
        }
        else {
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
                 getGlitzForDirection(frame->getView(), hg::FORWARDS),
                 timeEngine.getWall(),
                 resources,
                 wallImage);
        }
    }
    DrawTimeline(app.getRenderTarget(), timeEngine, waveInfo.updatedFrames, drawnFrame, timeEngine.getReplayData().back().getTimeCursor(), timeEngine.getTimelineLength());
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
            foreach (
                int num,
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
    /*{
        std::stringstream fpsstring;
        fpsstring << (1./app.GetFrameTime());
        sf::Text fpsglyph;
        fpsglyph.setFont(*hg::defaultFont);
        fpsglyph.setString(fpsstring.str());
        fpsglyph.setPosition(600, 465);
        fpsglyph.setCharacterSize(8.f);
        fpsglyph.setColor(uiTextColor);
        app.draw(fpsglyph);
    }*/
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
