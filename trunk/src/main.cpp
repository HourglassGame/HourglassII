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
typedef sf::Color Colour;

namespace {
    void initialseCurrentPath(std::vector<std::string> const& args);
    int run_main(std::vector<std::string> const& args);
    void runStep(
        hg::TimeEngine& timeEngine,
        hg::RenderWindow& app,
        hg::Inertia& inertia,
        hg::TimeEngine::RunResult const& waveInfo,
        hg::LevelResources const& resources,
        sf::Image const& wallImage);
    void Draw(
        hg::RenderWindow& target,
        hg::mt::std::vector<hg::Glitz>::type const& glitz,
        hg::Wall const& wall,
        hg::LevelResources const& resources,
        sf::Image const& wallImage);
    void DrawWaves(
        sf::RenderTarget& target,
        hg::TimeEngine::FrameListList const& waves,
        int timelineLength,
        double height);
    void DrawTimelineContents(
        sf::RenderTarget& target,
        hg::TimeEngine& timeEngine,
        double height);
    void DrawColours(hg::RenderWindow& target, int roomWidth, int roomHeight);
    void DrawTicks(sf::RenderTarget& target, std::size_t const timelineLength);
    void DrawTimeline(
        sf::RenderTarget& target,
        hg::TimeEngine& timeEngine,
        hg::TimeEngine::FrameListList const& waves,
        hg::FrameID playerFrame,
        hg::FrameID timeCursor,
        int timelineLength);
    //void DrawWall(sf::RenderTarget& target, hg::Wall const& wallData);
    template<typename BidirectionalGuyRange>
    hg::GuyOutputInfo const& findCurrentGuy(BidirectionalGuyRange const& guyRange);

    void saveReplayLog(std::ostream& toAppendTo, hg::InputList const& toAppend);
    void generateReplay();
    template<typename F>
    boost::unique_future<typename boost::result_of<F()>::type> enqueue_task(hg::ConcurrentQueue<hg::move_function<void()> >& queue, F f)
    {
        boost::packaged_task<typename boost::result_of<F()>::type> task(f);
        boost::unique_future<typename boost::result_of<F()>::type> future(task.get_future());
        queue.push(hg::move_function<void()>(boost::move(task)));
        return boost::move(future);
    }

    struct FunctionQueueRunner
    {
        FunctionQueueRunner(hg::ConcurrentQueue<hg::move_function<void()> >& taskQueue) :
            taskQueue_(taskQueue)
        {}
        void operator()() {
            while (!boost::this_thread::interruption_requested()) {
    			taskQueue_.pop()();
    		}
        }
    private:
        hg::ConcurrentQueue<hg::move_function<void()> >& taskQueue_;
    };

    struct RunToNextPlayerFrame
    {
        RunToNextPlayerFrame(hg::TimeEngine& timeEngine, hg::InputList const& inputList, hg::OperationInterrupter& interrupter) :
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

        CreateTimeEngine(std::string const& filename, hg::OperationInterrupter& interrupter)
            : filename(filename), interrupter(&interrupter) {}

        hg::TimeEngine operator()() const {
            return hg::TimeEngine(hg::loadLevelFromFile(filename, *interrupter), *interrupter);
        }
    private:
        std::string filename;
        hg::OperationInterrupter *interrupter;
    };
}


//Please note that main is the most horrible part of HourglassII at this time.
//The entire front end/UI is a massive pile of hacks mounted on hacks.
//On the other hand, code from TimeEngine downwards is generally well-designed
//and logical.

//Exceptions and HourglassII:
//Unless otherwise specified, all functions in HourglassII can throw std::bad_alloc.
//Unless otherwise specified, all functions in HourglassII provide the weak exception safety guarantee.
//The exact meaning of the above statement is somewhat ambiguous, so don't assume anything about objects
//which may have been modified by a function which has thrown an exception, without checking the documentation
//(please add documentation if it is missing).
//All exceptions other than std::bad_alloc should be explicitly documented, but this is not uniformly done.
int main(int argc, char *argv[])
{
    std::vector<std::string> args(argv, argv+argc);
    initialseCurrentPath(args);

    if(!hg::getTestDriver().passesAllTests()) {
        std::cerr << "Failed self-check! Aborting." << std::endl;
        return EXIT_FAILURE;
    }

    return run_main(args);
}

namespace  {
void initialseCurrentPath(std::vector<std::string> const& args)
{
#if defined(__APPLE__) && defined(__MACH__)
    assert(args.size() >= 1);
    current_path(boost::filesystem::path(args[0]).remove_filename()/"../Resources/");
#elif defined(_WIN32)
	assert(args.size() >= 1);
    current_path(boost::filesystem::path(args[0]).remove_filename()/"data/");
#endif
}

#if 0
struct AsyncWindow {
    
};

struct UserInterface {
    UserInterface():
        keepRunning(true)
    {}


//Used by game to control user interface:
    AsyncWindow createWindow(WindowSettings) {
        
    }
    
    InputStream getInputStream() {
        
    }

    void updateView(hg::ViewChange newView) {
        UpdateSounds(soundManager, newView.sounds);
        UpdateGraphics(graphicsManager, newView.graphics);
    }

    atomic<bool> keepRunning;


//Private
    void run() {
        while (keepRunning) {
            while (input = app.GetInput()) {
                inputList.push_back(input);
            }
            
            graphicsManager.drawTo(renderTarget);
        }
    }

    
    hg::View latestView;
    
    Window window; //Managing stuff relating to the window *other than* drawing and input.
    RenderTarget renderTarget;
    SoundManager soundManager;
    GraphicsManager graphicsManager;
    InputSource inputSource;
};

struct RunningGameView {

};

struct RunLevelOperation {

};

struct RunHourglassOperation {
    
};

int new_run_main(std::vector<std::string> const& args) {
    hg::UserInterface ui;
    boost::thread gameThread(run_game, ui);
    ui.run();
    gameThread.join();
    return 0;
}

static void run_game(hg::UserInterface& ui) {
    InputStream inputStream(ui.getInputStream());
    WindowHandle mainWindow(ui.createWindow(sf::VideoMode(640, 480), "Hourglass II"));

    GameState state;
    while (true) {
        ViewChange newView(UpdateGameState(state, inputList));

        ui.PlaySounds(sounds);
        FillView(newView);
        Sleep();
    }
}

struct LoadingGameScene {
    void update() {
        //if doneLoading {
        //goto nextState
        //}
        //if 
    }
    void draw();


    //Scene *previousScene; //Used if loading is cancelled
    //TimeEngine timeEngine;
    //Level level;
    //FileSystem files; //For the level's dependencies
    //RenderWindow window;
    //InputSource input;
    //OperationInterrupter interrupter;
};

struct RunningGameScene {
    
};
#endif

static hg::FrameID mousePosToFrameID(hg::RenderWindow const& app, hg::TimeEngine const& timeEngine) {
    int const timelineLength = timeEngine.getTimelineLength();
    double const mouseXFraction = app.getInputState().getMousePosition().x*1./app.getSize().x;
    int mouseFrame(hg::flooredModulo(static_cast<int>(mouseXFraction*timelineLength), timelineLength));
    return hg::FrameID(mouseFrame, hg::UniverseID(timelineLength));
}

int run_main(std::vector<std::string> const& args)
{
    hg::RenderWindow app(sf::VideoMode(640, 480), "Hourglass II");
    app.setVerticalSyncEnabled(true);
    app.setFramerateLimit(60);
    sf::Font defaultFontObj;
	bool fontLoaded(defaultFontObj.loadFromFile("Arial.ttf"));
    assert(fontLoaded);
    hg::defaultFont = &defaultFontObj;
    
    hg::ConcurrentQueue<hg::move_function<void()> > timeEngineTaskQueue;
    boost::thread timeEngineThread((FunctionQueueRunner(timeEngineTaskQueue)));

    std::string const levelPath("level.lvl");

    hg::unique_ptr<hg::OperationInterrupter> interrupter(new hg::OperationInterrupter());

    boost::unique_future<hg::TimeEngine> futureTimeEngine(
    	enqueue_task(
    		timeEngineTaskQueue,
    		CreateTimeEngine(levelPath, *interrupter)));

    //timeEngine would be a boost::optional if boost::optional supported r-value refs
    hg::unique_ptr<hg::TimeEngine> timeEngine;
    hg::LevelResources levelResources;
    sf::Image wallImage;

    enum {LOADING_LEVEL, LOADING_RESOURCES, RUNNING_LEVEL, AWAITING_INPUT, PAUSED} state(LOADING_LEVEL);

    hg::Input input;

    hg::Inertia inertia;
    std::vector<hg::InputList> replay;
    std::vector<hg::InputList>::const_iterator currentReplayIt(replay.begin());
    std::vector<hg::InputList>::const_iterator currentReplayEnd(replay.end());
    //Saves a replay continuously, but in a less nice format.
    //Gets rewritten whenever the level is reset.
    //Useful for tracking down crashes.
    std::ofstream replayLogOut("replayLogOut");
    boost::unique_future<hg::TimeEngine::RunResult> futureRunResult;
    bool runningFromReplay(false);
    while (app.isOpen()) {
    	switch (state) {
			case LOADING_LEVEL:
			{
                //Uses:   app
                //        interrupter
                //        futureTimeEngine
                //
                //Replaces: timeEngine
                //          input
                //          inertia
                //          interrupter
                {
                    sf::Event event;
                    while (app.pollEvent(event))
                    {
                        switch(event.type) {
                        case sf::Event::Closed:
                            interrupter->interrupt();
                            futureTimeEngine.wait();
                            app.close();
                            goto breakmainloop;
                        default: break;
                        }
                    }
                }
				if (futureTimeEngine.is_ready()) {
                    try {
                        timeEngine = hg::unique_ptr<hg::TimeEngine>(new hg::TimeEngine(futureTimeEngine.get()));
                        input.setTimelineLength(timeEngine->getTimelineLength());
                        inertia = hg::Inertia();
                        interrupter.reset();
                        state = LOADING_RESOURCES;
                    }
                    catch (hg::LuaError const& e) {
                        std::cerr << "There was an error in some lua, the error message was:\n" << e.message << std::endl;
						goto breakmainloop;
                    }
                    catch (std::bad_alloc const&) {
						std::cerr << "oops... ran out of memory ):" << std::endl;
						goto breakmainloop;
                    }
                    catch (std::exception const& e) {
                        std::cerr << e.what() << std::endl;
                        goto breakmainloop;
                    }
				}
				sf::Text loadingGlyph;
                loadingGlyph.setFont(*hg::defaultFont);
                loadingGlyph.setString("Loading Level...");
				loadingGlyph.setColor(Colour(255,255,255));
				loadingGlyph.setPosition(520, 450);
				loadingGlyph.setCharacterSize(12.f);
				app.clear();
				app.draw(loadingGlyph);
				app.display();
				break;
			}
            case LOADING_RESOURCES:
            {
                //Due to SFML limitations, resource loading must be done in the main window thread.
                //These limitations could be removed by weaning ourselves off SFML
                sf::Text loadingGlyph;
                loadingGlyph.setFont(*hg::defaultFont);
                loadingGlyph.setString("Loading Resources...");
				loadingGlyph.setColor(Colour(255,255,255));
				loadingGlyph.setPosition(520, 450);
				loadingGlyph.setCharacterSize(12);
				app.clear();
				app.draw(loadingGlyph);
				app.display();
                levelResources = hg::loadLevelResources(levelPath, "GlitzData");
                wallImage = hg::loadAndBakeWallImage(timeEngine->getWall());
                state = AWAITING_INPUT;
            }
			case AWAITING_INPUT:
			{
				hg::InputList inputList;
				if (currentReplayIt != currentReplayEnd) {
					inputList = *currentReplayIt;
					++currentReplayIt;
					runningFromReplay = true;
				}
				else {
                    hg::Wall const& wall(timeEngine->getWall());
                    double scalingFactor(std::max(wall.roomWidth()*1./app.getSize().x, wall.roomHeight()*1./app.getSize().y));
					input.updateState(app.getInputState(), app.getSize().x, scalingFactor);
					inputList = input.AsInputList();
					runningFromReplay = false;
				}
				saveReplayLog(replayLogOut, inputList);
                interrupter = hg::unique_ptr<hg::OperationInterrupter>(new hg::OperationInterrupter());
                futureRunResult =
                    enqueue_task(
                        timeEngineTaskQueue,
                        RunToNextPlayerFrame(*timeEngine, inputList, *interrupter));
				state = RUNNING_LEVEL;
				break;
			}
			case RUNNING_LEVEL:
			{
				sf::Event event;
				while (app.pollEvent(event))
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
						app.close();
						goto breakmainloop;
					case sf::Event::KeyPressed:
						switch(event.key.code) {
						//Restart
						case sf::Keyboard::R:
							currentReplayIt = replay.end();
							currentReplayEnd = replay.end();
							replayLogOut.close();
							replayLogOut.open("replayLogOut");
                            interrupter->interrupt();
							futureRunResult.wait();
                            interrupter = hg::unique_ptr<hg::OperationInterrupter>(new hg::OperationInterrupter());
						    futureTimeEngine =
						    	enqueue_task(
						    		timeEngineTaskQueue,
						    		CreateTimeEngine(levelPath, *interrupter));
							state = LOADING_LEVEL;
							goto continuemainloop;
						//Load replay
						case sf::Keyboard::L:
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
						    		timeEngineTaskQueue,
						    		CreateTimeEngine(levelPath, *interrupter));
							state = LOADING_LEVEL;
							goto continuemainloop;
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
							saveReplay("replay", timeEngine->getReplayData());
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
                    if (app.getInputState().isKeyPressed(sf::Keyboard::Period)) {
                        inertia.save(mousePosToFrameID(app, *timeEngine), hg::FORWARDS);
                    }
                    if (app.getInputState().isKeyPressed(sf::Keyboard::Comma)) {
                        inertia.save(mousePosToFrameID(app, *timeEngine), hg::REVERSE);
                    }
                    if (app.getInputState().isKeyPressed(sf::Keyboard::Slash)) {
                        inertia.reset();
                    }
					try {
						runStep(*timeEngine, app, inertia, futureRunResult.get(), levelResources, wallImage);
                        interrupter.reset();
					}
					catch (hg::PlayerVictoryException const&) {
						std::cout << "Congratulations, a winner is you" << std::endl;
						goto breakmainloop;
					}
                    catch (hg::LuaError const& e) {
                        std::cerr << "There was an error in some lua, the error message was:\n" << e.message << std::endl;
						goto breakmainloop;
                    }
					catch (std::bad_alloc const&) {
						std::cerr << "oops... ran out of memory ):" << std::endl;
						goto breakmainloop;
					}
					if (runningFromReplay) {
						sf::Text replayGlyph;
                        replayGlyph.setFont(*hg::defaultFont);
                        replayGlyph.setString("R");
						replayGlyph.setColor(Colour(255,0,0));
						replayGlyph.setPosition(580, 32);
						replayGlyph.setCharacterSize(32.f);
						app.draw(replayGlyph);
					}
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::F)) {
                        app.setFramerateLimit(0);
                        app.setVerticalSyncEnabled(false);
                    }
                    else {
                        app.setFramerateLimit(60);
                        app.setVerticalSyncEnabled(true);
                    }
					app.display();
					state = AWAITING_INPUT;
				}
                else {
                    sf::sleep(sf::seconds(.001f));
                }
				break;
			}
            case PAUSED:
            {
                {
                    sf::Event event;
                    while (app.pollEvent(event))
                    {
                        switch(event.type) {
                        case sf::Event::Closed:
                            interrupter->interrupt();
                            futureTimeEngine.wait();
                            app.close();
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
                sf::sleep(sf::seconds(.001f));
            }
    	}
    	continuemainloop:;
    }
    breakmainloop:
    timeEngineThread.interrupt();
    timeEngineThread.join();

    return EXIT_SUCCESS;
}

hg::mt::std::vector<hg::Glitz>::type const& getGlitzForDirection(
    hg::FrameView const& view, hg::TimeDirection timeDirection)
{
    return timeDirection == hg::FORWARDS ? view.getForwardsGlitz() : view.getReverseGlitz();
}

Colour const uiTextColour(100,100,200);

void drawInventory(hg::RenderWindow& app, hg::mt::std::map<hg::Ability, int>::type const& pickups, hg::Ability abilityCursor) {
    hg::mt::std::map<hg::Ability, int>::type mpickups(pickups);
    {
        std::stringstream timeJump;
        timeJump << (abilityCursor == hg::TIME_JUMP ? "-->" : "   ") << "timeJumps: " << mpickups[hg::TIME_JUMP];
        sf::Text timeJumpGlyph;
        timeJumpGlyph.setFont(*hg::defaultFont);
        timeJumpGlyph.setString(timeJump.str());
        timeJumpGlyph.setPosition(500, 350);
        timeJumpGlyph.setCharacterSize(10.f);
        timeJumpGlyph.setColor(uiTextColour);
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
        timeReversesGlyph.setColor(uiTextColour);
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
        timeGunsGlyph.setColor(uiTextColour);
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
        timePausesGlyph.setColor(uiTextColour);
        app.draw(timePausesGlyph);
    }
}

void runStep(
    hg::TimeEngine& timeEngine,
    hg::RenderWindow& app,
    hg::Inertia& inertia,
    hg::TimeEngine::RunResult const& waveInfo,
    hg::LevelResources const& resources,
    sf::Image const& wallImage)
{
    std::vector<int> framesExecutedList;
    hg::FrameID drawnFrame;

    framesExecutedList.reserve(boost::distance(waveInfo.updatedFrames()));
    foreach (
        hg::FrameUpdateSet const& updateSet,
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
        hg::Frame *frame(timeEngine.getFrame(drawnFrame));
        Draw(app,
             getGlitzForDirection(frame->getView(), hg::FORWARDS),
             timeEngine.getWall(),
             resources,
             wallImage);
    }
    else if (waveInfo.currentPlayerFrame()) {
        hg::FrameView const& view(waveInfo.currentPlayerFrame()->getView());
        hg::GuyOutputInfo const& currentGuy(findCurrentGuy(view.getGuyInformation()));
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
            hg::Frame *frame(timeEngine.getFrame(inertialFrame));
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
            hg::Frame *frame(timeEngine.getFrame(drawnFrame));
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
        currentPlayerGlyph.setColor(uiTextColour);
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
        frameNumberGlyph.setColor(uiTextColour);
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
        numberOfFramesExecutedGlyph.setColor(uiTextColour);
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
        fpsglyph.setColor(uiTextColour);
        app.draw(fpsglyph);
    }*/
}

void Draw(
    hg::RenderWindow& target,
    hg::mt::std::vector<hg::Glitz>::type const& glitz,
    hg::Wall const& wall,
    hg::LevelResources const& resources,
    sf::Image const& wallImage)
{
    target.clear(Colour(255,255,255));
    //Number by which all positions are be multiplied
    //to shrink or enlarge the level to the size of the
    //window.
    double scalingFactor(std::min(target.getSize().x*100./wall.roomWidth(), target.getSize().y*100./wall.roomHeight()));
    sf::View oldView(target.getView());
    sf::View scaledView(sf::FloatRect(0.f, 0.f, target.getSize().x/scalingFactor, target.getSize().y/scalingFactor));
    target.setView(scaledView);
    hg::sfRenderTargetCanvas canvas(target.getRenderTarget(), resources);
    hg::LayeredCanvas layeredCanvas(canvas);
	foreach (hg::Glitz const& particularGlitz, glitz) particularGlitz.display(layeredCanvas);
    hg::Flusher flusher(layeredCanvas.getFlusher());
    flusher.partialFlush(1000);
    sf::Texture wallTex;
    wallTex.loadFromImage(wallImage);
    target.draw(sf::Sprite(wallTex));
    //DrawWall(target, wall);
    flusher.partialFlush(std::numeric_limits<int>::max());
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) DrawColours(target, wall.roomWidth(), wall.roomHeight());
    target.setView(oldView);
}
/*
void DrawWall(
    sf::RenderTarget& target,
    hg::Wall const& wall)
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
                  Colour(50,50,50)));
            }
        }
    }
}
*/
Colour asColour(sf::Vector3<double>const& vec) {
    return Colour(vec.x, vec.y, vec.z);
}

Colour guyPositionToColour(double xFrac, double yFrac) {
    static sf::Vector3<double> const posStart(255,0,0);
    static sf::Vector3<double> const xMax(127,255,0);
    static sf::Vector3<double> const yMax(128,0,255);
    
    static sf::Vector3<double> const xDif(xMax - posStart);
    static sf::Vector3<double> const yDif(yMax - posStart);

    return asColour(posStart + xDif*xFrac + yDif*yFrac);
}

void DrawColours(hg::RenderWindow& target, int roomWidth, int roomHeight)
{
    sf::Image colours;
    colours.create(roomWidth/100, roomHeight/100, Colour(0, 0, 0, 0));
    for (int x(0); x != roomWidth/100; ++x) {
        for (int y(0); y != roomHeight/100; ++y) {
            Colour colour(guyPositionToColour(x*100./roomWidth,y*100./roomHeight));
            colour.a = 220;
            colours.setPixel(x, y, colour);
        }
    }
    sf::Texture tex;
    tex.loadFromImage(colours);
    
    target.draw(sf::Sprite(tex));
}

void DrawTimelineContents(
    sf::RenderTarget& target,
    hg::TimeEngine& timeEngine,
    double height)
{
    sf::Image timelineContents;
    timelineContents.create(target.getView().getSize().x, height, Colour(0, 0, 0, 0));
    double const numberOfGuys(timeEngine.getReplayData().size()+1);
    double const timelineLength(timeEngine.getTimelineLength());
    hg::UniverseID const universe(timeEngine.getTimelineLength());
    
    for (int frameNumber = 0, end = timeEngine.getTimelineLength(); frameNumber != end; ++frameNumber) {
        hg::Frame const *const frame(timeEngine.getFrame(getArbitraryFrame(universe, frameNumber)));
        foreach (hg::GuyOutputInfo const& guy, frame->getView().getGuyInformation()) {
            double left = frameNumber*target.getView().getSize().x/timelineLength;
            double top = (height-4)*guy.getIndex()/numberOfGuys;
            
            double xFrac = guy.getX()/static_cast<double>(timeEngine.getWall().roomWidth());
            double yFrac = guy.getY()/static_cast<double>(timeEngine.getWall().roomHeight());

            Colour const colour(guyPositionToColour(xFrac, yFrac));

            int pos(top);
            for (int const end(top+1); pos != end; ++pos) {
                timelineContents.setPixel(
                    left, pos,
                    !guy.getBoxCarrying() ?
                        colour :
                        guy.getBoxCarryDirection() == guy.getTimeDirection() ?
                            Colour(255, 0, 255)
                          : Colour(0, 255, 0));
            }
            for (int const end(top+4); pos != end; ++pos) {
                timelineContents.setPixel(left, pos, colour);
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
    sf::RenderTarget& target,
    hg::TimeEngine::FrameListList const& waves,
    int timelineLength,
    double height)
{
    std::vector<char> pixelsWhichHaveBeenDrawnIn(target.getView().getSize().x);
    foreach (hg::FrameUpdateSet const& wave, waves) {
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
                    wavegroup.setFillColor(Colour(250,0,0));
                    target.draw(wavegroup);
                    inWaveRegion = false;
                }
            }
        }
        //Draw when waves extend to far right.
        if (inWaveRegion) {
            sf::RectangleShape wavegroup(sf::Vector2f(target.getView().getSize().x-leftOfWaveRegion, height));
            wavegroup.setPosition(leftOfWaveRegion, 10.f);
            wavegroup.setFillColor(Colour(250,0,0));
            target.draw(wavegroup);
        }
    }
}

void DrawTicks(sf::RenderTarget& target, std::size_t const timelineLength) {
    for (std::size_t frameNo(0); frameNo < timelineLength; frameNo += 5*60) {
        float left(frameNo/static_cast<double>(timelineLength)*target.getView().getSize().x);
        sf::RectangleShape tick(sf::Vector2f(1., 10.));
        tick.setFillColor(Colour(255,255,0));
        tick.setPosition(sf::Vector2f(left, 0.));
        target.draw(tick);
    }
}

void DrawTimeline(
    sf::RenderTarget& target,
    hg::TimeEngine& timeEngine,
    hg::TimeEngine::FrameListList const& waves,
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
        playerLine.setFillColor(Colour(200,200,0));
        target.draw(playerLine);
    }
    if (timeCursor.isValidFrame()) {
        sf::RectangleShape timeCursorLine(sf::Vector2f(3, height));
        timeCursorLine.setPosition(timeCursor.getFrameNumber()*target.getView().getSize().x/timelineLength, 10.f);
        timeCursorLine.setFillColor(Colour(0,0,200));
        target.draw(timeCursorLine);
    }
    DrawTimelineContents(target, timeEngine, height);
}

struct CompareIndicies {
    template<typename IndexableType>
    bool operator()(IndexableType const& l, IndexableType const& r) {
        return l.getIndex() < r.getIndex();
    }
};

template<typename BidirectionalGuyRange>
hg::GuyOutputInfo const& findCurrentGuy(BidirectionalGuyRange const& guyRange)
{
    return *boost::begin(guyRange | boost::adaptors::reversed);
}


void saveReplayLog(std::ostream& toAppendTo, hg::InputList const& toAppend)
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
