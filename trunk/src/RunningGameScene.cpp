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

namespace hg {
sf::Color guyPositionToColor(double xFrac, double yFrac);
sf::Color asColor(sf::Vector3<double>const &vec);
void drawInventory(hg::RenderWindow &app, hg::mt::std::map<hg::Ability, int>::type const &pickups, hg::Ability abilityCursor);
hg::mt::std::vector<hg::Glitz>::type const &getGlitzForDirection(
    hg::FrameView const &view, hg::TimeDirection timeDirection);
void unsync_iostreams_with_stdio();
void initialseCurrentPath(std::vector<std::string> const &args);
int run_main(std::vector<std::string> const &args);
void runStep(
    hg::TimeEngine &timeEngine,
    hg::RenderWindow &app,
    hg::Inertia &inertia,
    hg::TimeEngine::RunResult const &waveInfo,
    hg::LevelResources const &resources,
    sf::Image const &wallImage);
void Draw(
    hg::RenderWindow &target,
    hg::mt::std::vector<hg::Glitz>::type const &glitz,
    hg::Wall const &wall,
    hg::LevelResources const &resources,
    sf::Image const &wallImage);
void DrawWaves(
    sf::RenderTarget &target,
    hg::TimeEngine::FrameListList const &waves,
    int timelineLength,
    double height);
void DrawTimelineContents(
    sf::RenderTarget &target,
    hg::TimeEngine &timeEngine,
    double height);
void DrawColors(hg::RenderWindow &target, int roomWidth, int roomHeight);
void DrawTicks(sf::RenderTarget &target, std::size_t const timelineLength);
void DrawTimeline(
    sf::RenderTarget &target,
    hg::TimeEngine &timeEngine,
    hg::TimeEngine::FrameListList const &waves,
    hg::FrameID playerFrame,
    hg::FrameID timeCursor,
    int timelineLength);
template<typename BidirectionalGuyRange>
hg::GuyOutputInfo const &findCurrentGuy(BidirectionalGuyRange const &guyRange);

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

static hg::FrameID mousePosToFrameID(hg::RenderWindow const &app, hg::TimeEngine const &timeEngine) {
    int const timelineLength = timeEngine.getTimelineLength();
    double const mouseXFraction = app.getInputState().getMousePosition().x*1./app.getSize().x;
    int mouseFrame(hg::flooredModulo(static_cast<int>(mouseXFraction*timelineLength), timelineLength));
    return hg::FrameID(mouseFrame, hg::UniverseID(timelineLength));
}

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

variant<WindowClosed_tag, GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>>
run_game_scene(hg::RenderWindow &window, LoadedLevel &&loadedLevel, std::vector<hg::InputList> const& replay)
{
    LoadedLevel const level(loadedLevel);

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
						return WindowClosed_tag{};
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
                            timeEngine = level.timeEngine;
                            levelResources = level.resources;
                            wallImage = level.bakedWall;
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
							//TODO - possible allow finer-grained access
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
						std::cout << "Congratulations, a winner is you" << std::endl;
                        run_post_level_scene(window, timeEngine);
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
    return WindowClosed_tag{};
}


hg::mt::std::vector<hg::Glitz>::type const &getGlitzForDirection(
    hg::FrameView const &view, hg::TimeDirection timeDirection)
{
    return timeDirection == hg::FORWARDS ? view.getForwardsGlitz() : view.getReverseGlitz();
}


sf::Color const uiTextColor(100,100,200);


void drawInventory(hg::RenderWindow &app, hg::mt::std::map<hg::Ability, int>::type const &pickups, hg::Ability abilityCursor) {
    hg::mt::std::map<hg::Ability, int>::type mpickups(pickups);
    {
        std::stringstream timeJump;
        timeJump << (abilityCursor == hg::TIME_JUMP ? "-->" : "   ") << "timeJumps: " << mpickups[hg::TIME_JUMP];
        sf::Text timeJumpGlyph;
        timeJumpGlyph.setFont(*hg::defaultFont);
        timeJumpGlyph.setString(timeJump.str());
        timeJumpGlyph.setPosition(500, 350);
        timeJumpGlyph.setCharacterSize(10.f);
        timeJumpGlyph.setColor(uiTextColor);
        app.draw(timeJumpGlyph);
    }
    {
        std::stringstream timeReverses;
        timeReverses << (abilityCursor == hg::TIME_REVERSE ? "-->" : "   ") << "timeReverses: " << mpickups[hg::TIME_REVERSE];
        sf::Text timeReversesGlyph;
        timeReversesGlyph.setFont(*hg::defaultFont);
        timeReversesGlyph.setString(timeReverses.str());
        timeReversesGlyph.setPosition(500, 370);
        timeReversesGlyph.setCharacterSize(10.f);
        timeReversesGlyph.setColor(uiTextColor);
        app.draw(timeReversesGlyph);
    }
    {
        std::stringstream timeGuns;
        timeGuns << (abilityCursor == hg::TIME_GUN ? "-->" : "   ") << "timeGuns: " << mpickups[hg::TIME_GUN];
        sf::Text timeGunsGlyph;
        timeGunsGlyph.setFont(*hg::defaultFont);
        timeGunsGlyph.setString(timeGuns.str());
        timeGunsGlyph.setPosition(500, 390);
        timeGunsGlyph.setCharacterSize(10.f);
        timeGunsGlyph.setColor(uiTextColor);
        app.draw(timeGunsGlyph);
    }
	{
        std::stringstream timeGuns;
        timeGuns << (abilityCursor == hg::TIME_PAUSE ? "-->" : "   ") << "timePauses: " << mpickups[hg::TIME_PAUSE];
        sf::Text timePausesGlyph;
        timePausesGlyph.setFont(*hg::defaultFont);
        timePausesGlyph.setString(timeGuns.str());
        timePausesGlyph.setPosition(500, 410);
        timePausesGlyph.setCharacterSize(10.f);
        timePausesGlyph.setColor(uiTextColor);
        app.draw(timePausesGlyph);
    }
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

    framesExecutedList.reserve(boost::distance(waveInfo.updatedFrames()));
    foreach (
        hg::FrameUpdateSet const &updateSet,
        waveInfo.updatedFrames())
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
        Draw(app,
             getGlitzForDirection(frame->getView(), hg::FORWARDS),
             timeEngine.getWall(),
             resources,
             wallImage);
    }
    else if (waveInfo.currentPlayerFrame()) {
        hg::FrameView const &view(waveInfo.currentPlayerFrame()->getView());
        hg::GuyOutputInfo const &currentGuy(findCurrentGuy(view.getGuyInformation()));
        hg::TimeDirection currentGuyDirection(currentGuy.getTimeDirection());
        inertia.save(hg::FrameID(waveInfo.currentPlayerFrame()), currentGuyDirection);
        drawnFrame = hg::FrameID(waveInfo.currentPlayerFrame());
        Draw(
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
            Draw(app,
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
            Draw(app,
                 getGlitzForDirection(frame->getView(), hg::FORWARDS),
                 timeEngine.getWall(),
                 resources,
                 wallImage);
        }
    }
    DrawTimeline(app.getRenderTarget(), timeEngine, waveInfo.updatedFrames(), drawnFrame, timeEngine.getReplayData().back().getTimeCursor(), timeEngine.getTimelineLength());
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


void Draw(
    hg::RenderWindow &target,
    hg::mt::std::vector<hg::Glitz>::type const &glitz,
    hg::Wall const &wall,
    hg::LevelResources const &resources,
    sf::Image const &wallImage)
{
    target.clear(sf::Color(255,255,255));
    //Number by which all positions are be multiplied
    //to shrink or enlarge the level to the size of the
    //window.
    double scalingFactor(std::min(target.getSize().x*100./wall.roomWidth(), target.getSize().y*100./wall.roomHeight()));
    sf::View oldView(target.getView());
    sf::View scaledView(sf::FloatRect(0.f, 0.f, target.getSize().x/scalingFactor, target.getSize().y/scalingFactor));
    target.setView(scaledView);
    hg::sfRenderTargetCanvas canvas(target.getRenderTarget(), resources);
    hg::LayeredCanvas layeredCanvas(canvas);
	foreach (hg::Glitz const &particularGlitz, glitz) particularGlitz.display(layeredCanvas);
    hg::Flusher flusher(layeredCanvas.getFlusher());
    flusher.partialFlush(1000);
    sf::Texture wallTex;
    wallTex.loadFromImage(wallImage);
    target.draw(sf::Sprite(wallTex));
    //DrawWall(target, wall);
    flusher.partialFlush(std::numeric_limits<int>::max());
    if (target.getInputState().isKeyPressed(sf::Keyboard::LShift)) DrawColors(target, wall.roomWidth(), wall.roomHeight());
    target.setView(oldView);
}

/*
void DrawWall(
    sf::RenderTarget &target,
    hg::Wall const &wall)
{
    for (int i(0), iend(wall.roomWidth()); i != iend; i += wall.segmentSize()) {
        for (int j(0), jend(wall.roomHeight()); j != jend; j += wall.segmentSize()) {
            if (wall.at(i, j)) {
              target.Draw(
                sf::Shape::Rectangle(
                  i/100.f,
                  j/100.f,
                  (i + wall.segmentSize())/100.f,
                  (j + wall.segmentSize())/100.f,
                  sf::Color(50,50,50)));
            }
        }
    }
}
*/

sf::Color asColor(sf::Vector3<double>const &vec) {
    return sf::Color(vec.x, vec.y, vec.z);
}


sf::Color guyPositionToColor(double xFrac, double yFrac) {
    static sf::Vector3<double> const posStart(255,0,0);
    static sf::Vector3<double> const xMax(127,255,0);
    static sf::Vector3<double> const yMax(128,0,255);
    
    static sf::Vector3<double> const xDif(xMax - posStart);
    static sf::Vector3<double> const yDif(yMax - posStart);

    return asColor(posStart + xDif*xFrac + yDif*yFrac);
}


void DrawColors(hg::RenderWindow &target, int roomWidth, int roomHeight)
{
    sf::Image colors;
    colors.create(roomWidth/100, roomHeight/100, sf::Color(0, 0, 0, 0));
    for (int x(0); x != roomWidth/100; ++x) {
        for (int y(0); y != roomHeight/100; ++y) {
            sf::Color color(guyPositionToColor(x*100./roomWidth,y*100./roomHeight));
            color.a = 220;
            colors.setPixel(x, y, color);
        }
    }
    sf::Texture tex;
    tex.loadFromImage(colors);
    
    target.draw(sf::Sprite(tex));
}


void DrawTimelineContents(
    sf::RenderTarget &target,
    hg::TimeEngine &timeEngine,
    double height)
{
    sf::Image timelineContents;
    timelineContents.create(target.getView().getSize().x, height, sf::Color(0, 0, 0, 0));
    double const numberOfGuys(timeEngine.getReplayData().size()+1);
    double const timelineLength(timeEngine.getTimelineLength());
    hg::UniverseID const universe(timeEngine.getTimelineLength());
    
    for (int frameNumber = 0, end = timeEngine.getTimelineLength(); frameNumber != end; ++frameNumber) {
        hg::Frame const *const frame(timeEngine.getFrame(getArbitraryFrame(universe, frameNumber)));
        foreach (hg::GuyOutputInfo const &guy, frame->getView().getGuyInformation()) {
            double left = frameNumber*target.getView().getSize().x/timelineLength;
            double top = (height-4)*guy.getIndex()/numberOfGuys;
            
            double xFrac = guy.getX()/static_cast<double>(timeEngine.getWall().roomWidth());
            double yFrac = guy.getY()/static_cast<double>(timeEngine.getWall().roomHeight());

            sf::Color const color(guyPositionToColor(xFrac, yFrac));

            int pos(top);
            for (int const end(top+1); pos != end; ++pos) {
                timelineContents.setPixel(
                    left, pos,
                    !guy.getBoxCarrying() ?
                        color :
                        guy.getBoxCarryDirection() == guy.getTimeDirection() ?
                            sf::Color(255, 0, 255)
                          : sf::Color(0, 255, 0));
            }
            for (int const end(top+4); pos != end; ++pos) {
                timelineContents.setPixel(left, pos, color);
            }
        }
    }
    sf::Texture tex;
    tex.loadFromImage(timelineContents);
    sf::Sprite sprite(tex);
    sprite.setPosition(0.f, 10.f);
    target.draw(sprite);
}


void DrawWaves(
    sf::RenderTarget &target,
    hg::TimeEngine::FrameListList const &waves,
    int timelineLength,
    double height)
{
    std::vector<char> pixelsWhichHaveBeenDrawnIn(target.getView().getSize().x);
    foreach (hg::FrameUpdateSet const &wave, waves) {
    	foreach (hg::Frame *frame, wave) {
            if (frame) {
                pixelsWhichHaveBeenDrawnIn[
                    static_cast<int>(
                        (static_cast<double>(getFrameNumber(frame))/timelineLength)
                        *target.getView().getSize().x)
                    ] = true;
            }
            else {
                assert(false && "I don't think that invalid frames can get updated");
            }
        }
        bool inWaveRegion = false;
        int leftOfWaveRegion = 0;
        for (int i = 0; i != static_cast<int>(pixelsWhichHaveBeenDrawnIn.size()); ++i) {
            bool pixelOn = pixelsWhichHaveBeenDrawnIn[i];
            if (pixelOn) {
                if (!inWaveRegion) {
                    leftOfWaveRegion = i;
                    inWaveRegion = true;
                }
            }
            else {
                if (inWaveRegion) {
                    sf::RectangleShape wavegroup(sf::Vector2f(i-leftOfWaveRegion, height));
                    wavegroup.setPosition(leftOfWaveRegion, 10.f);
                    wavegroup.setFillColor(sf::Color(250,0,0));
                    target.draw(wavegroup);
                    inWaveRegion = false;
                }
            }
        }
        //Draw when waves extend to far right.
        if (inWaveRegion) {
            sf::RectangleShape wavegroup(sf::Vector2f(target.getView().getSize().x-leftOfWaveRegion, height));
            wavegroup.setPosition(leftOfWaveRegion, 10.f);
            wavegroup.setFillColor(sf::Color(250,0,0));
            target.draw(wavegroup);
        }
    }
}


void DrawTicks(sf::RenderTarget &target, std::size_t const timelineLength) {
    for (std::size_t frameNo(0); frameNo < timelineLength; frameNo += 5*60) {
        float left(frameNo/static_cast<double>(timelineLength)*target.getView().getSize().x);
        sf::RectangleShape tick(sf::Vector2f(1., 10.));
        tick.setFillColor(sf::Color(255,255,0));
        tick.setPosition(sf::Vector2f(left, 0.));
        target.draw(tick);
    }
}


void DrawTimeline(
    sf::RenderTarget &target,
    hg::TimeEngine &timeEngine,
    hg::TimeEngine::FrameListList const &waves,
    hg::FrameID playerFrame,
    hg::FrameID timeCursor,
    int timelineLength)
{
    double height = 75.;
    
    DrawTicks(target, timelineLength);
    
    DrawWaves(target, waves, timelineLength, height);
    
    if (playerFrame.isValidFrame()) {
        sf::RectangleShape playerLine(sf::Vector2f(3, height));
        playerLine.setPosition(playerFrame.getFrameNumber()*target.getView().getSize().x/timelineLength, 10.f);
        playerLine.setFillColor(sf::Color(200,200,0));
        target.draw(playerLine);
    }
    if (timeCursor.isValidFrame()) {
        sf::RectangleShape timeCursorLine(sf::Vector2f(3, height));
        timeCursorLine.setPosition(timeCursor.getFrameNumber()*target.getView().getSize().x/timelineLength, 10.f);
        timeCursorLine.setFillColor(sf::Color(0,0,200));
        target.draw(timeCursorLine);
    }
    DrawTimelineContents(target, timeEngine, height);
}


struct CompareIndicies {
    template<typename IndexableType>
    bool operator()(IndexableType const &l, IndexableType const &r) {
        return l.getIndex() < r.getIndex();
    }
};


template<typename BidirectionalGuyRange>
hg::GuyOutputInfo const &findCurrentGuy(BidirectionalGuyRange const &guyRange)
{
    return *boost::begin(guyRange | boost::adaptors::reversed);
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
