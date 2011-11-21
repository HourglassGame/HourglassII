////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ObjectPtrList.h"
#include "ObjectListTypes.h"
#include "TimeEngine.h"
#include "PlayerVictoryException.h"
#include "Hg_Input.h"
#include "Level.h"
#include "Inertia.h"
#include "Frame.h"
#ifdef HG_COMPILE_TESTS
#include "TestDriver.h"
#endif //HG_COMPILE_TESTS
#include <SFML/Graphics.hpp>

#include <boost/multi_array.hpp>
#include <boost/assign.hpp>
#include "Foreach.h"
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/adaptor/sliced.hpp>
#include <boost/range/algorithm/max_element.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/irange.hpp>
#include <boost/range/istream_range.hpp>
#include <boost/filesystem.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <boost/serialization/vector.hpp>

#include <fstream>

#include <boost/bind.hpp>

#include <cstddef>
//#include <debug/vector>
#include <iostream>

#include <cmath>

#include "BasicConfiguredTriggerSystem.h"
#include "DirectLuaTriggerSystem.h"
#include "LevelLoader.h"

typedef sf::Color Colour;

using namespace hg;
using namespace std;
using namespace sf;
using namespace boost;
namespace {
    void initialseCurrentPath(int argc, char const* const argv[]);
    void runStep(TimeEngine& timeEngine, RenderWindow& app, Inertia& inertia, InputList const& input);
    void Draw(
        RenderWindow& target,
        mt::std::vector<Glitz>::type const& glitz,
        Wall const& wall);
    void DrawTimeline(RenderTarget& target, const TimeEngine::FrameListList& waves, FrameID playerFrame, std::size_t timelineLength);
    void DrawWall(RenderTarget& target, Wall const& wallData);
    void DrawGlitz(RenderTarget& target, mt::std::vector<Glitz>::type const& glitzList);
    template<typename BidirectionalGuyRange>
    TimeDirection findCurrentGuyDirection(const BidirectionalGuyRange& guyRange);
    
    std::vector<InputList> loadReplay();
    void saveReplay(std::vector<InputList> const& replay);
    void saveReplayLog(std::ostream& toAppendTo, InputList const& toAppend);
    void generateReplay();
}

int main(int argc, char const* const argv[])
{
#ifdef HG_COMPILE_TESTS
    if(!hg::getTestDriver().passesAllTests()) {
        return EXIT_FAILURE;
    }
#endif //HG_COMPILE_TESTS
    initialseCurrentPath(argc, argv);
    RenderWindow app(VideoMode(640, 480), "Hourglass II");
    app.UseVerticalSync(true);
    app.SetFramerateLimit(60);
    
    TimeEngine timeEngine(loadLevelFromFile("level.lua"));
    hg::Input input;
    input.setTimelineLength(timeEngine.getTimelineLength());
    hg::Inertia inertia;
    std::vector<InputList> replay;
    std::vector<InputList>::const_iterator currentReplayIt(replay.begin());
    std::vector<InputList>::const_iterator currentReplayEnd(replay.end());
    //Saves a replay continuously, but in a less nice format.
    //Gets rewritten whenever the level is reset.
    //Useful for tracking down crashes.
    std::ofstream replayLogOut("replayLogOut");
    while (app.IsOpened()) {
        Event event;
        while (app.GetEvent(event))
        {
            //States + transitions:
            //Not really a state machine!
            //Playing game -> new game + playing game               Keybinding: R
            //playing game -> new game + playig replay              Keybinding: L

            //playing replay -> new game + playing game             Keybinding: R
            //playing replay -> new game + playing replay           Keybinding: L
            //playing replay -> playing game                        Keybinding: P or <get to end of replay>
            switch (event.Type) {
            case sf::Event::Resized:
                std::cout << "resize event: width" << event.Size.Width << " height: " << event.Size.Height << "\n";
                break;
            case sf::Event::Closed:
                app.Close();
                goto breakmainloop;
            case sf::Event::KeyPressed:
                switch(event.Key.Code) {
                case sf::Key::R:
                    currentReplayIt = replay.end();
                    currentReplayEnd = replay.end();
                    replayLogOut.close();
                    replayLogOut.open("replayLogOut");
                    TimeEngine(loadLevelFromFile("level.lua")).swap(timeEngine);
                    input.setTimelineLength(timeEngine.getTimelineLength());
                    break;
                case sf::Key::L:
                    loadReplay().swap(replay);
                    currentReplayIt = replay.begin();
                    currentReplayEnd = replay.end();
                    replayLogOut.close();
                    replayLogOut.open("replayLogOut");
                    TimeEngine(loadLevelFromFile("level.lua")).swap(timeEngine);
                    input.setTimelineLength(timeEngine.getTimelineLength());
                    break;
                case sf::Key::P:
                    currentReplayIt = replay.end();
                    currentReplayEnd = replay.end();
                    break;
                case sf::Key::S:
                    saveReplay(timeEngine.getReplayData());
                    break;
                case sf::Key::G:
                    //Generate a replay from replayLogIn
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
        try {
            if (currentReplayIt != currentReplayEnd) {
                saveReplayLog(replayLogOut, *currentReplayIt);
                runStep(timeEngine, app, inertia, *currentReplayIt);
                ++currentReplayIt;
                sf::String replayGlyph("R");
                replayGlyph.SetColor(Colour(255,0,0));
                replayGlyph.SetPosition(580, 32);
                replayGlyph.SetSize(32.f);
                app.Draw(replayGlyph);
            }
            else {
                input.updateState(app.GetInput(), app.GetWidth());
                saveReplayLog(replayLogOut, input.AsInputList());
                runStep(timeEngine, app, inertia, input.AsInputList());
            }

            app.Display();
        }
        catch (hg::PlayerVictoryException&) {
            cout << "Congratulations, a winner is you!\n";
            return EXIT_SUCCESS;
        }
    }
    breakmainloop:
    return EXIT_SUCCESS;
}

namespace  {
void initialseCurrentPath(int, char const* const argv[])
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

void runStep(TimeEngine& timeEngine, RenderWindow& app, Inertia& inertia, InputList const& input)
{
    std::vector<std::size_t> framesExecutedList;
    FrameID drawnFrame;
    TimeEngine::RunResult const waveInfo(timeEngine.runToNextPlayerFrame(input));

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
    DrawWall(target, wall);
    DrawGlitz(target, glitz);
}

void DrawWall(
    sf::RenderTarget& target,
    Wall const& wall)
{
    target.Clear(Colour(0,0,0));
    
    for (int i(0), iend(wall.roomWidth()); i != iend; i += wall.segmentSize()) {
        for (int j(0), jend(wall.roomHeight()); j != jend; j += wall.segmentSize()) {
            if (!wall.at(i, j)) {
              target.Draw(
                sf::Shape::Rectangle(
                  i/100.f,
                  j/100.f,
                  (i + wall.segmentSize())/100.f,
                  (j + wall.segmentSize())/100.f,
                  Colour(255,255,255)));
            }
        }
    }
}


//Interprets colour as |...|RRRRRRRR|GGGGGGGG|BBBBBBBB|--------|
Colour interpretAsColour(unsigned colour)
{
    return Colour((colour & 0xFF000000) >> 24, (colour & 0xFF0000) >> 16, (colour & 0xFF00) >> 8);
}
void DrawParticularGlitz(RenderTarget& target, Glitz const& glitz)
{
    target.Draw(
        Shape::Rectangle(
            static_cast<float>(glitz.getX()/100),
            static_cast<float>(glitz.getY()/100),
            static_cast<float>((glitz.getX() + glitz.getWidth())/100),
            static_cast<float>((glitz.getY() + glitz.getHeight())/100),
            interpretAsColour(glitz.getColour())));
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
    foreach(FrameUpdateSet const& lists, waves) {
        foreach (Frame* frame, lists) {
            if (frame) {
                pixelsWhichHaveBeenDrawnIn[static_cast<std::size_t>((static_cast<double>(getFrameNumber(frame))/timelineLength)*target.GetView().GetRect().GetWidth())] = true;
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

//These are required to match each other, and to produce a 
//replay that can be saved on one machine and loaded on any other
//that is running HourglassII. That is, the replays myst be portable.
//The current implementations approximate this, but they should be improved!
std::vector<InputList> loadReplay()
{
    std::ifstream ifs("replay");
    std::vector<InputList> replay;
    if (ifs.is_open()) {
        boost::archive::text_iarchive ia(ifs);
        ia >> replay;
    }
    return replay;
}
void saveReplay(std::vector<InputList> const& replay)
{
    std::ofstream ofs("replay");
    boost::archive::text_oarchive oa(ofs);
    oa << replay;
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
        saveReplay(replay);
    }
}
}
