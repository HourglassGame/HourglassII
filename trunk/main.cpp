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
#include <boost/foreach.hpp>
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

typedef sf::Color Colour;
#define foreach BOOST_FOREACH
using namespace hg;
using namespace std;
using namespace sf;
using namespace boost;
namespace {
    void initialseCurrentPath(int argc, char const* const argv[]);
    void runStep(TimeEngine& timeEngine, RenderWindow& app, boost::multi_array<bool, 2> const& wall, Inertia& inertia, InputList const& input);
    void Draw(
        RenderWindow& target,
        ObjectPtrList<Normal> const& frame,
        mt::std::vector<RectangleGlitz>::type const& glitz,
        boost::multi_array<bool, 2> const& wall,
        TimeDirection playerDirection);
    void DrawTimeline(RenderTarget& target, const TimeEngine::FrameListList& waves, FrameID playerFrame);
    void DrawWall(RenderTarget& target, const boost::multi_array<bool, 2>& wallData);
    void DrawGlitz(RenderTarget& target, mt::std::vector<RectangleGlitz>::type const& glitz, TimeDirection playerDirection);
    template<typename RandomAccessBoxRange>
    void DrawBoxes(RenderTarget& target, const RandomAccessBoxRange& boxList, TimeDirection playerDirection);
    template<typename RandomAccessGuyRange>
    void DrawGuys(RenderTarget& target, const RandomAccessGuyRange& guyList, TimeDirection playerDirection);
    template<typename BidirectionalGuyRange>
    TimeDirection findCurrentGuyDirection(const BidirectionalGuyRange& guyRange);

    boost::multi_array<bool, 2> MakeWall();
    Level MakeLevel(const boost::multi_array<bool, 2>& wallData);
    
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
    
    boost::multi_array<bool, 2> const wall(MakeWall());
    TimeEngine timeEngine(MakeLevel(boost::multi_array<bool, 2>(wall)));
    hg::Input input;
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
                    TimeEngine(MakeLevel(wall)).swap(timeEngine);
                    break;
                case sf::Key::L:
                    loadReplay().swap(replay);
                    currentReplayIt = replay.begin();
                    currentReplayEnd = replay.end();
                    replayLogOut.close();
                    replayLogOut.open("replayLogOut");
                    TimeEngine(MakeLevel(wall)).swap(timeEngine);
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
                runStep(timeEngine, app, wall, inertia, *currentReplayIt);
                ++currentReplayIt;
                sf::String replayGlyph("R");
                replayGlyph.SetColor(Colour(255,0,0));
                replayGlyph.SetPosition(580, 32);
                replayGlyph.SetSize(32.f);
                app.Draw(replayGlyph);
            }
            else {
                input.updateState(app.GetInput());
                saveReplayLog(replayLogOut, input.AsInputList());
                runStep(timeEngine, app, wall, inertia, input.AsInputList());
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
void initialseCurrentPath(int argc, char const* const argv[])
{
#if defined(__APPLE__) && defined(__MACH__)
    assert(argc >= 1);
    current_path(boost::filesystem::path(argv[0]).remove_filename()/"../Resources/");
#endif
}

void runStep(TimeEngine& timeEngine, RenderWindow& app, boost::multi_array<bool, 2> const& wall, Inertia& inertia, InputList const& input)
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
            ObjectPtrList<Normal> const& frameData(waveInfo.currentPlayerFrame()->getPostPhysics());
            TimeDirection currentGuyDirection(findCurrentGuyDirection(frameData.getList<Guy>()));
            inertia.save(FrameID(waveInfo.currentPlayerFrame()), currentGuyDirection);
            drawnFrame = FrameID(waveInfo.currentPlayerFrame());
            Draw(
                app,
                frameData,
                waveInfo.currentPlayerFrame()->getGlitzFromHere(),
                wall,
                currentGuyDirection);
        }
        else {
            inertia.run();
            FrameID const inertialFrame(inertia.getFrame());
            if (inertialFrame.isValidFrame()) {
                drawnFrame = inertialFrame;
                Frame* frame(timeEngine.getFrame(inertialFrame));
                Draw(app, frame->getPostPhysics(), frame->getGlitzFromHere(), wall, inertia.getTimeDirection());
            }
            else {
                drawnFrame = FrameID(abs((app.GetInput().GetMouseX()*10800/640)%10800),UniverseID(10800));
                Frame* frame(timeEngine.getFrame(drawnFrame));
                Draw(app, frame->getPostPhysics(), frame->getGlitzFromHere(), wall, FORWARDS);
            }
        }
        DrawTimeline(app, waveInfo.updatedFrames(), drawnFrame);
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
    ObjectPtrList<Normal> const& frame,
    mt::std::vector<RectangleGlitz>::type const& glitz,
    boost::multi_array<bool, 2> const& wall,
    TimeDirection playerDirection)
{
    DrawWall(target, wall);
    DrawGlitz(target, glitz, playerDirection);
    DrawBoxes(target, frame.getList<Box>(), playerDirection);
    DrawGuys(target, frame.getList<Guy>(), playerDirection);
}

void DrawWall(
    sf::RenderTarget& target,
    boost::multi_array<bool, 2> const& wall)
{
    target.Clear(Colour(255,255,255));
    
    foreach (unsigned i, irange(0u, static_cast<unsigned>(wall.shape()[0]))) {
        foreach (unsigned j, irange(0u, static_cast<unsigned>(wall.shape()[1]))) {
            if (wall[i][j]) {
                target.Draw(
                    Shape::Rectangle(
                        32.f*i,
                        32.f*j,
                        32.f*(i+1),
                        32.f*(j+1),
                        Colour()));
            }
        }
    }
}


//Interprets colour as |...|RRRRRRRR|GGGGGGGG|BBBBBBBB|--------|
Colour interpretAsColour(unsigned colour)
{
    return Colour((colour & 0xFF000000) >> 24, (colour & 0xFF0000) >> 16, (colour & 0xFF00) >> 8);
}

void DrawParticularGlitz(RenderTarget& target, RectangleGlitz const& rectangleGlitz, TimeDirection playerDirection)
{
        if (playerDirection == rectangleGlitz.getTimeDirection()) {
            target.Draw(
                Shape::Rectangle(
                    static_cast<float>(rectangleGlitz.getX()/100),
                    static_cast<float>(rectangleGlitz.getY()/100),
                    static_cast<float>((rectangleGlitz.getX() + rectangleGlitz.getWidth())/100),
                    static_cast<float>((rectangleGlitz.getY() + rectangleGlitz.getHeight())/100),
                    interpretAsColour(rectangleGlitz.getForwardsColour())));
        }
        else {
            int const x(rectangleGlitz.getX() - rectangleGlitz.getXspeed());
            int const y(rectangleGlitz.getY() - rectangleGlitz.getYspeed());
            target.Draw(
                Shape::Rectangle(
                    static_cast<float>(x/100),
                    static_cast<float>(y/100),
                    static_cast<float>((x + rectangleGlitz.getWidth())/100),
                    static_cast<float>((y + rectangleGlitz.getHeight())/100),
                    interpretAsColour(rectangleGlitz.getReverseColour())));
        }
}

void DrawGlitz(RenderTarget& target, mt::std::vector<RectangleGlitz>::type const& glitz, TimeDirection playerDirection)
{
    foreach (RectangleGlitz const& rectangleGlitz, glitz) {
        DrawParticularGlitz(target, rectangleGlitz, playerDirection);
    }
}

template<typename RandomAccessBoxRange>
void DrawBoxes(
    RenderTarget& target,
    RandomAccessBoxRange const& boxList,
    TimeDirection const playerDirection)
{
    foreach(Box const& box, boxList) {
        //see below (in DrawGuys)
        //if (box.getArrivalBasis() == -1) {
            if (playerDirection == box.getTimeDirection()) {
                target.Draw(
                    Shape::Rectangle(
                        static_cast<float>(box.getX()/100),
                        static_cast<float>(box.getY()/100),
                        static_cast<float>((box.getX()+ box.getSize())/100),
                        static_cast<float>((box.getY()+box.getSize())/100),
                        Colour(255,0,255)));
            }
            else {
                int const x(box.getX()-box.getXspeed());
                int const y(box.getY()-box.getYspeed());
                target.Draw(
                    Shape::Rectangle(
                        static_cast<float>(x/100),
                        static_cast<float>(y/100),
                        static_cast<float>((x + box.getSize())/100),
                        static_cast<float>((y + box.getSize())/100),
                        Colour(0,255,0)));
            }
      //  }
    }
}

struct PositionAndColour
{ 
    PositionAndColour(int nx, int ny, Colour const& ncolour) :
        x(nx), y(ny), colour(ncolour) {}
    int x; int y; Colour colour;
};

template<typename RandomAccessGuyRange>
void DrawGuys(
    RenderTarget& target,
    RandomAccessGuyRange const& guyList,
    TimeDirection const playerDirection)
{
    foreach(Guy const& guy, guyList) {
        //Doesn't seem necessary -- could you give an example where strange stuff happens? Did this get fixed by flicker fix?
        //if (guy.getArrivalBasis() == -1) // if it is drawn when going through portal it may be somewhere strange, use same workaround as end of pause time flicker
        {
            
            PositionAndColour const pnc(
                playerDirection == guy.getTimeDirection() ? 
                    PositionAndColour(
                        guy.getX(),
                        guy.getY(),
                        Colour(150,150,0)) : 
                    PositionAndColour(
                        guy.getX() - guy.getXspeed(),
                        guy.getY() - guy.getYspeed(),
                        Colour(0,0,150)));
            
            int const left(pnc.x);
            int const top(pnc.y);
            int const vmid(pnc.y+guy.getHeight()/2);
            int const hmid(pnc.x+guy.getWidth()/2);
            int const right(pnc.x+guy.getWidth());
            int const bottom(pnc.y+guy.getHeight());
            
            target.Draw(
                Shape::Rectangle(
                    static_cast<float>(left/100),
                    static_cast<float>(top/100),
                    static_cast<float>(right/100),
                    static_cast<float>(bottom/100),
                    pnc.colour));
            
            target.Draw(
                guy.getFacing() ?
                    Shape::Rectangle(
                        static_cast<float>(hmid/100),
                        static_cast<float>(top/100),
                        static_cast<float>(right/100),
                        static_cast<float>(vmid/100),
                        Colour(50,50,50)) :
                    Shape::Rectangle(
                        static_cast<float>(left/100),
                        static_cast<float>(top/100),
                        static_cast<float>(hmid/100),
                        static_cast<float>(vmid/100),
                        Colour(50,50,50)));
            
            if (guy.getBoxCarrying()) {
                target.Draw(
                    Shape::Rectangle(
                        static_cast<float>((hmid - guy.getBoxCarrySize()/2)/100),
                        static_cast<float>((top - guy.getBoxCarrySize())/100),
                        static_cast<float>((hmid + guy.getBoxCarrySize()/2)/100),
                        static_cast<float>(top/100),
                        playerDirection == guy.getBoxCarryDirection() ?
                            Colour(150,0,150) :
                            Colour(0,150,0)));
            }
        }
    }
}

void DrawTimeline(
    RenderTarget& target,
    TimeEngine::FrameListList const& waves,
    FrameID const playerFrame)
{
    bool pixelsWhichHaveBeenDrawnIn[640] = {};
    foreach(FrameUpdateSet const& lists, waves) {
        foreach (Frame* frame, lists) {
            if (frame) {
                pixelsWhichHaveBeenDrawnIn[static_cast<std::size_t>(getFrameNumber(frame)/10800.*640)] = true;
            }
            else {
                assert(false && "I don't think that invalid frames can get updated");
            }
        }
        bool inWaveRegion = false;
        int leftOfWaveRegion = 0;
        for (int i = 0; i != sizeof(pixelsWhichHaveBeenDrawnIn) / sizeof(bool); ++i) {
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
                    static_cast<float>(640),
                    25.f,
                    Colour(250,0,0)));
        }
    }
    if (playerFrame.isValidFrame()) {
        target.Draw(
            Shape::Rectangle(
                static_cast<float>(static_cast<int>(playerFrame.getFrameNumber()/10800.*640-1)),
                10.f,
                static_cast<float>(static_cast<int>(playerFrame.getFrameNumber()/10800.*640+2)),
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
    return boost::begin(
        guyRange 
        | boost::adaptors::reversed)->getTimeDirection();
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

boost::multi_array<bool, 2> MakeWall()
{
    using namespace boost::assign;
    vector<vector<char> > wall;
    vector<char> row;

#if defined E || defined D
#error this madness has gone on quite long enough
#endif
#define E row +=
#define D ; wall.push_back(row); row.clear();
    E 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 D
    E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 D
    E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 D
    E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 D
    E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 D
    E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 D
    E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 D
    E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 D
    E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 D
    E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 D
    E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1 D
    E 1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1 D
    E 1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1 D
    E 1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1 D
    E 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 D
#undef D
#undef E
    //.at() used in case `wall' becomes non-square or 0 size.
    boost::array<boost::multi_array<bool, 2>::index, 2> const wallShape = {{ wall.at(0).size(), wall.size() }};
    boost::multi_array<bool, 2> actualWall(wallShape);
    for (unsigned int i = 0; i < wall.size(); ++i) {
        for (unsigned int j = 0; j < wall.at(i).size(); ++j) {
            actualWall[j][i] = wall.at(i).at(j);
        }
    }
    return actualWall;
}

TriggerSystem makeBasicConfiguredTriggerSystem()
{
    std::vector<ProtoPortal> protoPortals;
    protoPortals.push_back(
        ProtoPortal(
            Attachment(0,-4200,-3200),
            0,
            4200,
            4200,
            FORWARDS,
            0,
            0,
            -16000,
            true,
            120,
            0,
            true,
            false));
    
    std::vector<ProtoPlatform> protoPlatforms;
    protoPlatforms.push_back(
        ProtoPlatform(
            6400,
            1600,
            FORWARDS,
            1,
            0,
            PlatformDestinationPair(
                PlatformDestination(
                    PlatformDestinationComponent(
                        22400,
                        200,
                        50,
                        50),
                    PlatformDestinationComponent(
                        43800,
                        300,
                        50,
                        50)),
                PlatformDestination(
                    PlatformDestinationComponent(
                        38400,
                        200,
                        50,
                        50),
                    PlatformDestinationComponent(
                        43800,
                        300,
                        20,
                        20)))));
    
    std::vector<ProtoButton> protoButtons;
    protoButtons.push_back(
        ProtoButton(
            Attachment(0,3200,-800),
            3200,
            800,
            FORWARDS,
            0));
    
    std::vector<std::pair<int, std::vector<int> > > triggerOffsetsAndDefaults;
    triggerOffsetsAndDefaults.push_back(std::make_pair(1, std::vector<int>(1)));
    
    std::vector<int> defaultPlatformPositionAndVelocity;
    defaultPlatformPositionAndVelocity.push_back(38400);
    defaultPlatformPositionAndVelocity.push_back(43800);
    defaultPlatformPositionAndVelocity.push_back(0);
    defaultPlatformPositionAndVelocity.push_back(0);
    triggerOffsetsAndDefaults.push_back(std::make_pair(1, defaultPlatformPositionAndVelocity));
    
    return TriggerSystem(
        new BasicConfiguredTriggerSystem(
            protoPortals,
            protoPlatforms,
            protoButtons,
            triggerOffsetsAndDefaults));
}

TriggerSystem makeDirectLuaTriggerSystem()
{
    std::vector<char> triggerSystemLuaChunk;
    std::ifstream file;
    file.exceptions(std::ifstream::badbit | std::ifstream::failbit | std::ifstream::eofbit);
    file.open("triggerSystem.lua");
    file.seekg(0, std::ios::end);
    std::streampos length(file.tellg());
    if (length) {
        file.seekg(0, std::ios::beg);
        triggerSystemLuaChunk.resize(static_cast<std::size_t>(length));
        file.read(&triggerSystemLuaChunk.front(), static_cast<std::size_t>(length));
    }
    
    std::vector<std::pair<int, std::vector<int> > > triggerOffsetsAndDefaults;
    triggerOffsetsAndDefaults.push_back(std::make_pair(1, std::vector<int>(1)));
    
    std::vector<int> defaultPlatformPositionAndVelocity;
    defaultPlatformPositionAndVelocity.push_back(38400);
    defaultPlatformPositionAndVelocity.push_back(43800);
    defaultPlatformPositionAndVelocity.push_back(0);
    defaultPlatformPositionAndVelocity.push_back(0);
    triggerOffsetsAndDefaults.push_back(std::make_pair(1, defaultPlatformPositionAndVelocity));
    return TriggerSystem(
        new DirectLuaTriggerSystem(
            triggerSystemLuaChunk,
            triggerOffsetsAndDefaults,
            1));

}

Level MakeLevel(boost::multi_array<bool, 2> const& wall)
{
    ObjectList<NonGuyDynamic> newObjectList;

    newObjectList.add(Box(32400, 8000, 0, -600, 3200, -1, -1, FORWARDS));
    newObjectList.add(Box(46400, 14200, -1000, -500, 3200, -1, -1, FORWARDS));
    newObjectList.add(Box(46400, 10800, -1000, -500, 3200, -1, -1, FORWARDS));
    newObjectList.add(Box(46400, 17600, -1000, -500, 3200, -1, -1, FORWARDS));
    newObjectList.add(Box(46400, 21600, -500, -500, 3200, -1, -1, FORWARDS));
    newObjectList.add(Box(6400, 15600, 1000, -500, 3200, -1, -1, FORWARDS));
    newObjectList.add(Box(56400, 15600, 0, 0, 3200, -1, -1, FORWARDS));
    
    newObjectList.sort();
    
    return
        Level(
            3,
            10800,
            Environment(
                Wall(
                    3200,
                    wall),
                30),
            newObjectList,
            Guy(8700, 20000, 0, 0, 1600, 3200, 0, -1, false, 0, mt::std::map<int,int>::type(), false, false, 0, INVALID, FORWARDS, 0),
            FrameID(0,UniverseID(10800)),
            makeDirectLuaTriggerSystem());
}
}
