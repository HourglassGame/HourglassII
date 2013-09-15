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
    void runStep(hg::TimeEngine& timeEngine, sf::RenderWindow& app, hg::Inertia& inertia, hg::TimeEngine::RunResult const& waveInfo);
    void Draw(
        sf::RenderWindow& target,
        hg::mt::std::vector<hg::Glitz>::type const& glitz,
        hg::Wall const& wall);
    void DrawWaves(
        sf::RenderTarget& target,
        hg::TimeEngine::FrameListList const& waves,
        int timelineLength,
        double height);
    void DrawTimelineContents(
        sf::RenderTarget& target,
        hg::TimeEngine& timeEngine,
        double height);
    void DrawTimeline(
        sf::RenderTarget& target,
        hg::TimeEngine& timeEngine,
        hg::TimeEngine::FrameListList const& waves,
        hg::FrameID playerFrame,
        hg::FrameID timeCursor,
        int timelineLength);
    void DrawWall(sf::RenderTarget& target, hg::Wall const& wallData);
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

        CreateTimeEngine(hg::OperationInterrupter& interrupter) : interrupter_(&interrupter) {}

        hg::TimeEngine operator()() const {
            return hg::TimeEngine(hg::loadLevelFromFile("level.lvl", *interrupter_), *interrupter_);
        }
    private:
        hg::OperationInterrupter *interrupter_;
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

    if (false) {
        //To remove "unsed function" warning
        //return new_run_main(args);
        //return run_main(args);
    }
    return run_main(args);
}

namespace  {
void initialseCurrentPath(std::vector<std::string> const& args)
{
#if defined(__APPLE__) && defined(__MACH__)
    assert(args.size() >= 1);
    current_path(boost::filesystem::path(args[0]).remove_filename()/"../Resources/");
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
    InputStream inputStream(ui.getInputStream);
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

int run_main(std::vector<std::string> const& args)
{
    sf::RenderWindow app(sf::VideoMode(640, 480), "Hourglass II");
    app.UseVerticalSync(true);
    app.SetFramerateLimit(60);

    hg::ConcurrentQueue<hg::move_function<void()> > timeEngineTaskQueue;
    boost::thread timeEngineThread((FunctionQueueRunner(timeEngineTaskQueue)));

    hg::unique_ptr<hg::OperationInterrupter> interrupter(new hg::OperationInterrupter());

    boost::unique_future<hg::TimeEngine> futureTimeEngine(
    	enqueue_task(
    		timeEngineTaskQueue,
    		CreateTimeEngine(*interrupter)));

    //timeEngine would be a boost::optional if boost::optional supported r-value refs
    hg::unique_ptr<hg::TimeEngine> timeEngine;

    enum {LOADING_LEVEL, RUNNING_LEVEL, AWAITING_INPUT, PAUSED} state(LOADING_LEVEL);

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
    while (app.IsOpened()) {
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
                    while (app.GetEvent(event))
                    {
                        switch(event.Type) {
                        case sf::Event::Closed:
                            interrupter->interrupt();
                            futureTimeEngine.wait();
                            app.Close();
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
                        state = AWAITING_INPUT;
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
				sf::String loadingGlyph("Loading Level...");
				loadingGlyph.SetColor(Colour(255,255,255));
				loadingGlyph.SetPosition(520, 450);
				loadingGlyph.SetSize(12.f);
				app.Clear();
				app.Draw(loadingGlyph);
				app.Display();
				break;
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
                    double scalingFactor(std::max(wall.roomWidth()*1./app.GetWidth(), wall.roomHeight()*1./app.GetHeight()));
					input.updateState(app.GetInput(), app.GetWidth(), scalingFactor);
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
				while (app.GetEvent(event))
				{
					//States + transitions:
					//Not really a state machine!
					//Playing game -> new game + playing game               Keybinding: R
					//playing game -> new game + playing replay             Keybinding: L

					//playing replay -> new game + playing game             Keybinding: R
					//playing replay -> new game + playing replay           Keybinding: L
					//playing replay -> playing game                        Keybinding: C or <get to end of replay>
					switch (event.Type) {
					case sf::Event::Closed:
                        interrupter->interrupt();
                        futureRunResult.wait();
						app.Close();
						goto breakmainloop;
					case sf::Event::KeyPressed:
						switch(event.Key.Code) {
						//Restart
						case sf::Key::R:
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
						    		CreateTimeEngine(*interrupter));
							state = LOADING_LEVEL;
							goto continuemainloop;
						//Load replay
						case sf::Key::L:
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
						    		CreateTimeEngine(*interrupter));
							state = LOADING_LEVEL;
							goto continuemainloop;
						//Interrupt replay and begin Playing
						case sf::Key::C:
							currentReplayIt = replay.end();
							currentReplayEnd = replay.end();
							break;
						//Save replay
						case sf::Key::K:
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
						case sf::Key::G:
							generateReplay();
							break;
                        case sf::Key::P:
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
					try {
						runStep(*timeEngine, app, inertia, futureRunResult.get());
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
						sf::String replayGlyph("R");
						replayGlyph.SetColor(Colour(255,0,0));
						replayGlyph.SetPosition(580, 32);
						replayGlyph.SetSize(32.f);
						app.Draw(replayGlyph);
					}
                    if (app.GetInput().IsKeyDown(sf::Key::F)) {
                        app.SetFramerateLimit(0);
                        app.UseVerticalSync(false);
                    }
                    else {
                        app.SetFramerateLimit(60);
                        app.UseVerticalSync(true);
                    }
					app.Display();
					state = AWAITING_INPUT;
				}
                else {
                    sf::Sleep(.001f);
                }
				break;
			}
            case PAUSED:
            {
                {
                    sf::Event event;
                    while (app.GetEvent(event))
                    {
                        switch(event.Type) {
                        case sf::Event::Closed:
                            interrupter->interrupt();
                            futureTimeEngine.wait();
                            app.Close();
                            goto breakmainloop;
                        case sf::Event::KeyPressed:
						    switch(event.Key.Code) {
                            case sf::Key::P:
                                state = RUNNING_LEVEL;
                                break;
                            default: break;
                            }
                        default: break;
                        }
                    }
                }
                sf::Sleep(.001f);
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

void drawInventory(sf::RenderWindow& app, hg::mt::std::map<hg::Ability, int>::type const& pickups, hg::Ability abilityCursor) {
    hg::mt::std::map<hg::Ability, int>::type mpickups(pickups);
    {
        std::stringstream timeJump;
        timeJump << (abilityCursor == hg::TIME_JUMP ? "-->" : "   ") << " timeJumps: " << mpickups[hg::TIME_JUMP];
        sf::String timeJumpGlyph(timeJump.str());
        timeJumpGlyph.SetPosition(500, 350);
        timeJumpGlyph.SetSize(10.f);
        timeJumpGlyph.SetColor(uiTextColour);
        app.Draw(timeJumpGlyph);
    }
    {
        std::stringstream timeReverses;
        timeReverses << (abilityCursor == hg::TIME_REVERSE ? "-->" : "   ") << "timeReverses: " << mpickups[hg::TIME_REVERSE];
        sf::String timeReversesGlyph(timeReverses.str());
        timeReversesGlyph.SetPosition(500, 370);
        timeReversesGlyph.SetSize(10.f);
        timeReversesGlyph.SetColor(uiTextColour);
        app.Draw(timeReversesGlyph);
    }
    {
        std::stringstream timeGuns;
        timeGuns << (abilityCursor == hg::TIME_GUN ? "-->" : "   ") << "timeGuns: " << mpickups[hg::TIME_GUN];
        sf::String timeGunsGlyph(timeGuns.str());
        timeGunsGlyph.SetPosition(500, 390);
        timeGunsGlyph.SetSize(10.f);
        timeGunsGlyph.SetColor(uiTextColour);
        app.Draw(timeGunsGlyph);
    }
	{
        std::stringstream timeGuns;
        timeGuns << (abilityCursor == hg::TIME_PAUSE ? "-->" : "   ") << "timePauses: " << mpickups[hg::TIME_PAUSE];
        sf::String timePausesGlyph(timeGuns.str());
        timePausesGlyph.SetPosition(500, 410);
        timePausesGlyph.SetSize(10.f);
        timePausesGlyph.SetColor(uiTextColour);
        app.Draw(timePausesGlyph);
    }
}

void runStep(hg::TimeEngine& timeEngine, sf::RenderWindow& app, hg::Inertia& inertia, hg::TimeEngine::RunResult const& waveInfo)
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

    if (waveInfo.currentPlayerFrame()) {
        hg::FrameView const& view(waveInfo.currentPlayerFrame()->getView());
        hg::GuyOutputInfo const& currentGuy(findCurrentGuy(view.getGuyInformation()));
        hg::TimeDirection currentGuyDirection(currentGuy.getTimeDirection());
        inertia.save(hg::FrameID(waveInfo.currentPlayerFrame()), currentGuyDirection);
        drawnFrame = hg::FrameID(waveInfo.currentPlayerFrame());
        Draw(
            app,
            getGlitzForDirection(view, currentGuyDirection),
            timeEngine.getWall());
        
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
                 timeEngine.getWall());
        }
        else {
            drawnFrame =
              hg::FrameID(
                abs(
                  static_cast<int>(
                    hg::flooredModulo(static_cast<long>((app.GetInput().GetMouseX()
                     * static_cast<long>(timeEngine.getTimelineLength())
                     / app.GetWidth()))
                    , static_cast<long>(timeEngine.getTimelineLength())))),
                hg::UniverseID(timeEngine.getTimelineLength()));
            hg::Frame *frame(timeEngine.getFrame(drawnFrame));
            Draw(app,
                 getGlitzForDirection(frame->getView(), hg::FORWARDS),
                 timeEngine.getWall());
        }
    }
    DrawTimeline(app, timeEngine, waveInfo.updatedFrames(), drawnFrame, timeEngine.getReplayData().back().getTimeCursor(), timeEngine.getTimelineLength());
    {
        std::stringstream currentPlayerIndex;
        currentPlayerIndex << "Index: " << timeEngine.getReplayData().size() - 1;
        sf::String currentPlayerGlyph(currentPlayerIndex.str());
        currentPlayerGlyph.SetPosition(580, 433);
        currentPlayerGlyph.SetSize(10.f);
        currentPlayerGlyph.SetColor(uiTextColour);
        app.Draw(currentPlayerGlyph);
    }
    {
        std::stringstream frameNumberString;
        frameNumberString << "Frame: " << drawnFrame.getFrameNumber();
        sf::String frameNumberGlyph(frameNumberString.str());
        frameNumberGlyph.SetPosition(580, 445);
        frameNumberGlyph.SetSize(8.f);
        frameNumberGlyph.SetColor(uiTextColour);
        app.Draw(frameNumberGlyph);
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
        sf::String numberOfFramesExecutedGlyph(numberOfFramesExecutedString.str());
        numberOfFramesExecutedGlyph.SetPosition(580, 455);
        numberOfFramesExecutedGlyph.SetSize(8.f);
        numberOfFramesExecutedGlyph.SetColor(uiTextColour);
        app.Draw(numberOfFramesExecutedGlyph);
    }
    {
        std::stringstream fpsstring;
        fpsstring << (1./app.GetFrameTime());
        sf::String fpsglyph(fpsstring.str());
        fpsglyph.SetPosition(600, 465);
        fpsglyph.SetSize(8.f);
        fpsglyph.SetColor(uiTextColour);
        app.Draw(fpsglyph);
    }
}

void Draw(
    sf::RenderWindow& target,
    hg::mt::std::vector<hg::Glitz>::type const& glitz,
    hg::Wall const& wall)
{
    target.Clear(Colour(255,255,255));
    //Number by which all positions are be multiplied
    //to shrink or enlarge the level to the size of the
    //window.
    double scalingFactor(std::min(target.GetWidth()*100./wall.roomWidth(), target.GetHeight()*100./wall.roomHeight()));
    sf::View const& oldView(target.GetView());
    sf::View scaledView(sf::FloatRect(0.f, 0.f, target.GetWidth()/scalingFactor, target.GetHeight()/scalingFactor));
    target.SetView(scaledView);
    hg::sfRenderTargetCanvas canvas(target);
    hg::LayeredCanvas layeredCanvas(canvas);
	foreach (hg::Glitz const& particularGlitz, glitz) particularGlitz.display(layeredCanvas);
    hg::Flusher flusher(layeredCanvas.getFlusher());
    flusher.partialFlush(1000);
    DrawWall(target, wall);
    flusher.partialFlush(std::numeric_limits<int>::max());
    target.SetView(oldView);
}

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


void DrawTimelineContents(
    sf::RenderTarget& target,
    hg::TimeEngine& timeEngine,
    double height)
{
    sf::Image timelineContents(target.GetView().GetRect().GetWidth(), height, Colour(0, 0, 0, 0));
    double numberOfGuys(timeEngine.getReplayData().size()+1);
    double timelineLength(timeEngine.getTimelineLength());
    hg::UniverseID universe(timeEngine.getTimelineLength());
    for(int frameNumber = 0, end = timeEngine.getTimelineLength(); frameNumber != end; ++frameNumber) {
        hg::Frame *frame(timeEngine.getFrame(getArbitraryFrame(universe, frameNumber)));
        foreach (hg::GuyOutputInfo const& guy, frame->getView().getGuyInformation()) {
            double left = frameNumber*target.GetView().GetRect().GetWidth()/timelineLength;
            double top = height*guy.getIndex()/numberOfGuys;
            
            
            double x = hg::clamp(0., 255.*guy.getX()/timeEngine.getWall().roomWidth(), 255.);
            double y = hg::clamp(0., 255.*guy.getY()/timeEngine.getWall().roomHeight(), 255.);
            if (guy.getBoxCarrying())
            {
            	timelineContents.SetPixel(left, top, guy.getBoxCarryDirection() == guy.getTimeDirection() ? Colour(255, x, y) : Colour(0, x, y));
            }
            else
            {
            	 timelineContents.SetPixel(left, top, Colour(128,x,y));
            }
        }
    }
    target.Draw(sf::Sprite(timelineContents, sf::Vector2f(0, 10.f)));
}

void DrawWaves(
    sf::RenderTarget& target,
    hg::TimeEngine::FrameListList const& waves,
    int timelineLength,
    double height)
{
    std::vector<char> pixelsWhichHaveBeenDrawnIn(target.GetView().GetRect().GetWidth());
    foreach (hg::FrameUpdateSet const& wave, waves) {
    	foreach (hg::Frame *frame, wave) {
            if (frame) {
                pixelsWhichHaveBeenDrawnIn[
                    static_cast<int>(
                        (static_cast<double>(getFrameNumber(frame))/timelineLength)
                        *target.GetView().GetRect().GetWidth())
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
                    target.Draw(
                        sf::Shape::Rectangle(
                            static_cast<float>(leftOfWaveRegion),
                            10.f,
                            static_cast<float>(i),
                            10.f+height,
                            Colour(250,0,0)));
                    inWaveRegion = false;
                }
            }
        }
        //Draw when waves extend to far right.
        if (inWaveRegion) {
            target.Draw(
                sf::Shape::Rectangle(
                    static_cast<float>(leftOfWaveRegion),
                    10.f,
                    static_cast<float>(target.GetView().GetRect().GetWidth()),
                    10.f+height,
                    Colour(250,0,0)));
        }
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
    DrawTimelineContents(target, timeEngine, height);
    
    DrawWaves(target, waves, timelineLength, height);
    
    if (playerFrame.isValidFrame()) {
        target.Draw(
            sf::Shape::Rectangle(
                static_cast<float>(static_cast<int>(static_cast<double>(playerFrame.getFrameNumber())/timelineLength*target.GetView().GetRect().GetWidth()-1)),
                10.f,
                static_cast<float>(static_cast<int>(static_cast<double>(playerFrame.getFrameNumber())/timelineLength*target.GetView().GetRect().GetWidth()+2)),
                10.f+height,
                Colour(200,200,0)));
    }
    if (timeCursor.isValidFrame()) {
        target.Draw(
            sf::Shape::Rectangle(
                static_cast<float>(static_cast<int>(static_cast<double>(timeCursor.getFrameNumber())/timelineLength*target.GetView().GetRect().GetWidth()-1)),
                10.f,
                static_cast<float>(static_cast<int>(static_cast<double>(timeCursor.getFrameNumber())/timelineLength*target.GetView().GetRect().GetWidth()+2)),
                10.f+height,
                Colour(0,0,200)));
    }
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
