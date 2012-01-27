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
//#include <debug/vector>
#include <iostream>
#include <utility>

#include <cmath>

#include "BasicConfiguredTriggerSystem.h"
#include "DirectLuaTriggerSystem.h"
#include "LevelLoader.h"
#include "ConcurrentQueue.h"
#include "move_function.h"
#include "unique_ptr.h"
#include "sfRenderTargetCanvas.h"
#include "sfColour.h"

typedef sf::Color Colour;

using namespace hg;
using namespace std;
using namespace sf;
using namespace boost;
namespace {
    void initialseCurrentPath(int argc, char const* const* argv);
    void runStep(TimeEngine& timeEngine, RenderWindow& app, Inertia& inertia, TimeEngine::RunResult const& waveInfo);
    void Draw(
        RenderWindow& target,
        mt::std::vector<Glitz>::type const& glitz,
        Wall const& wall);
    void DrawTimeline(RenderTarget& target, const TimeEngine::FrameListList& waves, FrameID playerFrame, std::size_t timelineLength);
    void DrawWall(RenderTarget& target, Wall const& wallData);
    void DrawGlitz(RenderTarget& target, mt::std::vector<Glitz>::type const& glitzList);
    template<typename BidirectionalGuyRange>
    TimeDirection findCurrentGuyDirection(BidirectionalGuyRange const& guyRange);
    
    void saveReplayLog(std::ostream& toAppendTo, InputList const& toAppend);
    void generateReplay();
    template<typename F>
    boost::unique_future<typename boost::result_of<F()>::type> enqueue_task(ConcurrentQueue<move_function<void()> >& queue, F f)
    {
        boost::packaged_task<typename boost::result_of<F()>::type> task(f);
        boost::unique_future<typename boost::result_of<F()>::type> future(task.get_future());
        queue.push(move_function<void()>(boost::move(task)));
        return boost::move(future);
    }

    struct FunctionQueueRunner
    {
        FunctionQueueRunner(ConcurrentQueue<move_function<void()> >& taskQueue) :
            taskQueue_(taskQueue)
        {}
        void operator()() {
            while (!boost::this_thread::interruption_requested()) {
    			taskQueue_.pop()();
    		}
        }
    private:
        ConcurrentQueue<move_function<void()> >& taskQueue_;
    };

    struct RunToNextPlayerFrame
    {
        RunToNextPlayerFrame(TimeEngine& timeEngine, InputList const& inputList) :
            timeEngine_(timeEngine),
            inputList_(inputList)
        {}
        typedef TimeEngine::RunResult result_type;
        TimeEngine::RunResult operator()()
        {
            return timeEngine_.runToNextPlayerFrame(boost::move(inputList_));
        }
    private:
        TimeEngine& timeEngine_;
        InputList inputList_;
    };

    TimeEngine createTimeEngine() {
        return TimeEngine(loadLevelFromFile("level.lua"));
    }
}


//Please note that main is the most horrible part of HourglassII at this time.
//The entire front end/UI is a massive pile of hacks mounted on hacks.
//On the other hand, code from TimeEngine downwards is generally well-designed
//and logical.
int main(int argc, char* argv[])
{
    if(!hg::getTestDriver().passesAllTests()) {
        std::cerr << "Failed self-check! Aborting." << std::endl;
        return EXIT_FAILURE;
    }

    initialseCurrentPath(argc, argv);

    RenderWindow app(VideoMode(640, 480), "Hourglass II");
    app.UseVerticalSync(true);
    app.SetFramerateLimit(60);
    ProgressMonitor monitor;

    ConcurrentQueue<move_function<void()> > timeEngineTaskQueue;
    boost::thread timeEngineThread((FunctionQueueRunner(timeEngineTaskQueue)));

    boost::unique_future<TimeEngine> futureTimeEngine(
    	enqueue_task(
    		timeEngineTaskQueue,
    		createTimeEngine));

    //timeEngine would be a boost::optional if boost::optional supported r-value refs
    hg::unique_ptr<TimeEngine> timeEngine;

    enum {LOADING_LEVEL, RUNNING_LEVEL, AWAITING_INPUT} state(LOADING_LEVEL);

    hg::Input input;

    hg::Inertia inertia;
    std::vector<InputList> replay;
    std::vector<InputList>::const_iterator currentReplayIt(replay.begin());
    std::vector<InputList>::const_iterator currentReplayEnd(replay.end());
    //Saves a replay continuously, but in a less nice format.
    //Gets rewritten whenever the level is reset.
    //Useful for tracking down crashes.
    std::ofstream replayLogOut("replayLogOut");
    boost::unique_future<TimeEngine::RunResult> futureRunResult;
    bool runningFromReplay(false);
    while (app.IsOpened()) {
    	switch (state) {
			case LOADING_LEVEL:
			{
            /*
				Event event;
				while (app.GetEvent(event))
				{
					switch(event.Type) {
						//TODO - if (closed or canceled) {cancel_loading(); exit();}
                        default: break;
					}
				}*/
				if (futureTimeEngine.is_ready()) {
                    try {
                        timeEngine = hg::unique_ptr<TimeEngine>(new TimeEngine(futureTimeEngine.get()));
                        input.setTimelineLength(timeEngine->getTimelineLength());
                        state = AWAITING_INPUT;
                    } catch(std::bad_alloc const&) {
						std::cerr << "oops... ran out of memory ):" << std::endl;
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
				InputList inputList;
				if (currentReplayIt != currentReplayEnd) {
					inputList = *currentReplayIt;
					++currentReplayIt;
					runningFromReplay = true;
				}
				else {
					input.updateState(app.GetInput(), app.GetWidth());
					inputList = input.AsInputList();
					runningFromReplay = false;
				}
				saveReplayLog(replayLogOut, inputList);
                futureRunResult =
                    enqueue_task(
                        timeEngineTaskQueue,
                        RunToNextPlayerFrame(*timeEngine, inputList));
				state = RUNNING_LEVEL;
				break;
			}
			case RUNNING_LEVEL:
			{
				Event event;
				while (app.GetEvent(event))
				{
					//States + transitions:
					//Not really a state machine!
					//Playing game -> new game + playing game               Keybinding: R
					//playing game -> new game + playing replay             Keybinding: L

					//playing replay -> new game + playing game             Keybinding: R
					//playing replay -> new game + playing replay           Keybinding: L
					//playing replay -> playing game                        Keybinding: P or <get to end of replay>
					switch (event.Type) {
					case sf::Event::Closed:
						//TODO - add something which cancels the execution of the time engine
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
							//TODO - cancel time engine execution
							futureRunResult.wait();
						    futureTimeEngine =
						    	enqueue_task(
						    		timeEngineTaskQueue,
						    		createTimeEngine);
							state = LOADING_LEVEL;
							goto continuemainloop;
						//Load replay
						case sf::Key::L:
							replay = loadReplay("replay");
							currentReplayIt = replay.begin();
							currentReplayEnd = replay.end();
							replayLogOut.close();
							replayLogOut.open("replayLogOut");
							//TODO - cancel time engine execution
							futureRunResult.wait();
						    futureTimeEngine =
						    	enqueue_task(
						    		timeEngineTaskQueue,
						    		createTimeEngine);
							state = LOADING_LEVEL;
							goto continuemainloop;
						//Interrupt replay and begin Playing
						case sf::Key::P:
							currentReplayIt = replay.end();
							currentReplayEnd = replay.end();
							break;
						//Save replay
						case sf::Key::S:
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
					}
					catch (PlayerVictoryException const&) {
						std::cout << "Congratulations, a winner is you" << std::endl;
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
                    Sleep(.001f);
                }
				break;
			}
    	}
    	continuemainloop:
    	;
    }

    breakmainloop:
    timeEngineThread.interrupt();
    timeEngineThread.join();

    return EXIT_SUCCESS;
}

namespace  {
void initialseCurrentPath(int argc, char const* const* argv)
{
#if defined(__APPLE__) && defined(__MACH__)
    assert(argc >= 1);
    current_path(boost::filesystem::path(argv[0]).remove_filename()/"../Resources/");
#endif
}

mt::std::vector<Glitz>::type const& getGlitzForDirection(FrameView const& view, TimeDirection timeDirection)
{
    return timeDirection == FORWARDS ? view.getForwardsGlitz() : view.getReverseGlitz();
}

void runStep(TimeEngine& timeEngine, RenderWindow& app, Inertia& inertia, TimeEngine::RunResult const& waveInfo)
{
    std::vector<std::size_t> framesExecutedList;
    FrameID drawnFrame;

    framesExecutedList.reserve(boost::distance(waveInfo.updatedFrames()));
    foreach (
        FrameUpdateSet const& updateSet,
        waveInfo.updatedFrames())
    {
        framesExecutedList.push_back(boost::distance(updateSet));
    }
    
    if (waveInfo.currentPlayerFrame()) {
        FrameView const& view(waveInfo.currentPlayerFrame()->getView());
        TimeDirection currentGuyDirection(findCurrentGuyDirection(view.getGuyInformation()));
        inertia.save(FrameID(waveInfo.currentPlayerFrame()), currentGuyDirection);
        drawnFrame = FrameID(waveInfo.currentPlayerFrame());
        Draw(
            app,
            getGlitzForDirection(view, currentGuyDirection),
            timeEngine.getWall());
    }
    else {
        inertia.run();
        FrameID const inertialFrame(inertia.getFrame());
        if (inertialFrame.isValidFrame()) {
            drawnFrame = inertialFrame;
            Frame* frame(timeEngine.getFrame(inertialFrame));
            Draw(app,
                 getGlitzForDirection(frame->getView(), inertia.getTimeDirection()),
                 timeEngine.getWall());
        }
        else {
            drawnFrame =
              FrameID(
                abs(
                  static_cast<int>(
                    (app.GetInput().GetMouseX() * static_cast<long>(timeEngine.getTimelineLength()) / app.GetWidth())
                     % static_cast<long>(timeEngine.getTimelineLength()))),
                UniverseID(timeEngine.getTimelineLength()));
            Frame* frame(timeEngine.getFrame(drawnFrame));
            Draw(app,
                 getGlitzForDirection(frame->getView(), FORWARDS),
                 timeEngine.getWall());
        }
    }
    DrawTimeline(app, waveInfo.updatedFrames(), drawnFrame, timeEngine.getTimelineLength());
    {
        stringstream currentPlayerIndex;
        currentPlayerIndex << "Index: " << timeEngine.getReplayData().size() - 1;
        sf::String currentPlayerGlyph(currentPlayerIndex.str());
        currentPlayerGlyph.SetPosition(580, 433);
        currentPlayerGlyph.SetSize(10.f);
        app.Draw(currentPlayerGlyph);
    }
    {
        stringstream frameNumberString;
        frameNumberString << "Frame: " << drawnFrame.getFrameNumber();
        sf::String frameNumberGlyph(frameNumberString.str());
        frameNumberGlyph.SetPosition(580, 445);
        frameNumberGlyph.SetSize(8.f);
        app.Draw(frameNumberGlyph);
    }
    {
        stringstream numberOfFramesExecutedString;
        if (!boost::empty(framesExecutedList)) {
            numberOfFramesExecutedString << *boost::begin(framesExecutedList);
            foreach (
                std::size_t num,
                framesExecutedList 
                | boost::adaptors::sliced(1, boost::distance(framesExecutedList)))
            {
                numberOfFramesExecutedString << ":" << num;
            }
        }
        sf::String numberOfFramesExecutedGlyph(numberOfFramesExecutedString.str());
        numberOfFramesExecutedGlyph.SetPosition(580, 455);
        numberOfFramesExecutedGlyph.SetSize(8.f);
        app.Draw(numberOfFramesExecutedGlyph);
    }
    {
        stringstream fpsstring;
        fpsstring << (1./app.GetFrameTime());
        sf::String fpsglyph(fpsstring.str());
        fpsglyph.SetPosition(600, 465);
        fpsglyph.SetSize(8.f);
        app.Draw(fpsglyph);
    }
}

void Draw(
    RenderWindow& target,
    mt::std::vector<Glitz>::type const& glitz,
    Wall const& wall)
{
    target.Clear(Colour(255,255,255));
    //Number by which all positions are be multiplied
    //to shrink or enlarge the level to the size of the
    //window.
    double scalingFactor(std::min(target.GetWidth()*100./wall.roomWidth(), target.GetHeight()*100./wall.roomHeight()));
    sf::View const& oldView(target.GetView());
    sf::View scaledView(sf::FloatRect(0.f, 0.f, target.GetWidth()/scalingFactor, target.GetHeight()/scalingFactor));
    target.SetView(scaledView);
    DrawGlitz(target, glitz);
    DrawWall(target, wall);
    target.SetView(oldView);
}

void DrawWall(
    sf::RenderTarget& target,
    Wall const& wall)
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

void DrawParticularGlitz(RenderTarget& target, Glitz const& glitz)
{
    sfRenderTargetCanvas canvas(target);
    glitz.display(canvas);
}
void DrawGlitz(RenderTarget& target, mt::std::vector<Glitz>::type const& glitzList)
{
	foreach (Glitz const& glitz, glitzList) DrawParticularGlitz(target, glitz);
}

void DrawTimeline(
    RenderTarget& target,
    TimeEngine::FrameListList const& waves,
    FrameID const playerFrame,
    std::size_t timelineLength)
{
    std::vector<char> pixelsWhichHaveBeenDrawnIn(target.GetView().GetRect().GetWidth());
    foreach (FrameUpdateSet const& wave, waves) {
    	foreach (Frame* frame, wave) {
            if (frame) {
                pixelsWhichHaveBeenDrawnIn[
                    static_cast<std::size_t>(
                        (static_cast<double>(getFrameNumber(frame))/timelineLength)
                        *target.GetView().GetRect().GetWidth())
                    ] = true;
            }
            else {
                assert(false && "I don't think that invalid frames can get updated");
            }
        }
        bool inWaveRegion = false;
        std::size_t leftOfWaveRegion = 0;
        for (std::size_t i = 0; i != pixelsWhichHaveBeenDrawnIn.size(); ++i) {
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
                        Shape::Rectangle(
                            static_cast<float>(leftOfWaveRegion),
                            10.f,
                            static_cast<float>(i),
                            25.f,
                            Colour(250,0,0)));
                    inWaveRegion = false;
                }
            }
        }
        //Draw when waves extend to far right.
        if (inWaveRegion) {
            target.Draw(
                Shape::Rectangle(
                    static_cast<float>(leftOfWaveRegion),
                    10.f,
                    static_cast<float>(target.GetView().GetRect().GetWidth()),
                    25.f,
                    Colour(250,0,0)));
        }
    }
    if (playerFrame.isValidFrame()) {
        target.Draw(
            Shape::Rectangle(
                static_cast<float>(static_cast<int>(static_cast<double>(playerFrame.getFrameNumber())/timelineLength*target.GetView().GetRect().GetWidth()-1)),
                10.f,
                static_cast<float>(static_cast<int>(static_cast<double>(playerFrame.getFrameNumber())/timelineLength*target.GetView().GetRect().GetWidth()+2)),
                25.f,
                Colour(200,200,0)));
    }
}

struct CompareIndicies {
    template<typename IndexableType>
    bool operator()(IndexableType const& l, IndexableType const& r) {
        return l.getIndex() < r.getIndex();
    }
};

template<typename BidirectionalGuyRange>
TimeDirection findCurrentGuyDirection(BidirectionalGuyRange const& guyRange)
{
    return boost::begin(guyRange | boost::adaptors::reversed)->getTimeDirection();
}


void saveReplayLog(std::ostream& toAppendTo, InputList const& toAppend)
{
    toAppendTo << toAppend << " " << std::flush;
}
void generateReplay()
{
    std::ifstream replayLogIn("replayLogIn");
    if (replayLogIn.is_open()) {
        std::vector<InputList> replay;
        replay.assign(std::istream_iterator<InputList>(replayLogIn), std::istream_iterator<InputList>());
        saveReplay("replay", replay);
    }
}
}
