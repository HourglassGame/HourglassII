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

#include <boost/bind.hpp>

#include <cstddef>
//#include <debug/vector>
#include <iostream>

#include <cmath>
#include "nedmalloc.h"
//#include "tbb/scalable_allocator.h"
#include <new>

typedef sf::Color Colour;

void* custom_malloc(std::size_t size);
void custom_free(void* p);

void* custom_malloc(std::size_t size)
{
    //puts("m");
    return nedalloc::nedmalloc(size);
    //return scalable_malloc(size);
}

void custom_free(void* p)
{
    //puts("f");
    //scalable_free(p);
    //free(p);
    if (p) {
        nedalloc::nedfree(p);
    }
}

void* operator new(std::size_t size) throw(std::bad_alloc)
{
    while (true) {
        void* pointer(custom_malloc(size));
        if (pointer) {
            return pointer;
        }
        else if (std::new_handler handler = std::set_new_handler(0)) {
            std::set_new_handler(handler);
            (*handler)();
        }
        else {
            throw std::bad_alloc();
        }
    }
}

void operator delete(void *p) throw()
{
    custom_free(p);
}

void* operator new(std::size_t size, std::nothrow_t const&) throw()
{
    while (true) {
        void* pointer(custom_malloc(size));
        if (pointer) {
            return pointer;
        }
        else if (std::new_handler handler = std::set_new_handler(0)) {
            std::set_new_handler(handler);
            (*handler)();
        }
        else {
            throw 0;
        }
    }
}

void operator delete(void *p, std::nothrow_t const&) throw()
{
    custom_free(p);
}

void* operator new[](std::size_t size) throw(std::bad_alloc)
{
    return ::operator new(size);
}

void operator delete[](void *p) throw()
{
    ::operator delete(p);
}

void* operator new[](std::size_t size, std::nothrow_t const& nothrow) throw()
{
    return ::operator new(size, nothrow);
}

void operator delete[](void *p, std::nothrow_t const& nothrow) throw()
{
    ::operator delete(p, nothrow);
}

#define foreach BOOST_FOREACH
using namespace hg;
using namespace std;
using namespace sf;
using namespace boost;
namespace {
    void Draw(RenderWindow& target, const ObjectPtrList<Normal> & frame, const boost::multi_array<bool, 2>& wall, TimeDirection playerDirection);
    void DrawTimeline(RenderTarget& target, const TimeEngine::FrameListList& waves, FrameID playerFrame);
    void DrawWall(RenderTarget& target, const boost::multi_array<bool, 2>& wallData);
    template<typename RandomAccessBoxRange>
    void DrawBoxes(RenderTarget& target, const RandomAccessBoxRange& boxList, TimeDirection playerDirection);
    template<typename RandomAccessGuyRange>
    void DrawGuys(RenderTarget& target, const RandomAccessGuyRange& guyList, TimeDirection playerDirection);
    template<typename RandomAccessButtonRange>
    void DrawButtons(RenderTarget& target, const RandomAccessButtonRange& buttonList, TimeDirection playerDirection);
    template<typename RandomAccessPlatformRange>
    void DrawPlatforms(RenderTarget& target, const RandomAccessPlatformRange& platformList, TimeDirection playerDirection);
    template<typename RandomAccessPortalRange>
    void DrawPortals(RenderTarget& target, const RandomAccessPortalRange& portalList, TimeDirection playerDirection);
    template<typename BidirectionalGuyRange>
    TimeDirection findCurrentGuyDirection(const BidirectionalGuyRange& guyRange);

    boost::multi_array<bool, 2> MakeWall();
    Level MakeLevel(const boost::multi_array<bool, 2>& wallData);
}

int main()
{
#ifdef HG_COMPILE_TESTS
    if(!hg::getTestDriver().passesAllTests()) {
        return EXIT_FAILURE;
    }
#endif //HG_COMPILE_TESTS
    RenderWindow app(VideoMode(640, 480), "Hourglass II");
    app.UseVerticalSync(true);
    app.SetFramerateLimit(60);
    boost::multi_array<bool, 2> const wall(MakeWall());
    TimeEngine timeEngine(MakeLevel(wall));
    hg::Input input;
    hg::Inertia inertia;
    while (app.IsOpened())
    {
        Event event;
        while (app.GetEvent(event))
        {
            switch (event.Type) {
            case sf::Event::Closed:
                app.Close();
                break;
            default:
                break;
            }
        }
        input.updateState(app.GetInput());
        //cout << "called from main" << endl;
        std::vector<std::size_t> framesExecutedList;
        try {
            FrameID drawnFrame;
            TimeEngine::RunResult const waveInfo(timeEngine.runToNextPlayerFrame(input.AsInputList()));
            boost::push_back(
                framesExecutedList,
                waveInfo.updatedFrames() 
                    | boost::adaptors::transformed(
                        boost::bind(boost::distance<FrameUpdateSet>, _1)));
            
            if (waveInfo.currentPlayerFrame()) {
                ObjectPtrList<Normal> const& frameData(waveInfo.currentPlayerFrame()->getPostPhysics());
                TimeDirection currentGuyDirection(findCurrentGuyDirection(frameData.getList<Guy>()));
                inertia.save(FrameID(waveInfo.currentPlayerFrame()), currentGuyDirection);
                drawnFrame = FrameID(waveInfo.currentPlayerFrame());
                Draw(
                    app,
                    frameData,
                    wall,
                    currentGuyDirection);
            }
            else {
                inertia.run();
                FrameID const inertialFrame(inertia.getFrame());
                drawnFrame = inertialFrame;
                if (inertialFrame.isValidFrame()) {
                    Draw(app, timeEngine.getFrame(inertialFrame)->getPostPhysics(), wall, inertia.getTimeDirection());
                }
                else {
                    Draw(app, timeEngine.getFrame(FrameID(abs((app.GetInput().GetMouseX()*10800/640)%10800),UniverseID(10800)))->getPostPhysics(), wall, FORWARDS);
                }
            }
            DrawTimeline(app, waveInfo.updatedFrames(), drawnFrame);
        }
        catch (hg::PlayerVictoryException& playerWon) {
            cout << "Congratulations, a winner is you!\n";
            return EXIT_SUCCESS;
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
            numberOfFramesExecutedGlyph.SetPosition(580, 445);
            numberOfFramesExecutedGlyph.SetSize(8.f);
            app.Draw(numberOfFramesExecutedGlyph);
        }
        {
            stringstream memorystring;
            memorystring << nedalloc::nedmalloc_footprint() << "B";
            sf::String memoryglyph(memorystring.str());
            memoryglyph.SetPosition(580, 455);
            memoryglyph.SetSize(8.f);
            app.Draw(memoryglyph);
        }
        {
            stringstream fpsstring;
            fpsstring << (1./app.GetFrameTime());
            sf::String fpsglyph(fpsstring.str());
            fpsglyph.SetPosition(600, 465);
            fpsglyph.SetSize(8.f);
            app.Draw(fpsglyph);
        }
        app.Display();
    }
    return EXIT_SUCCESS;
}

namespace  {
void Draw(
    RenderWindow& target,
    ObjectPtrList<Normal> const& frame,
    boost::multi_array<bool, 2> const& wall,
    TimeDirection const playerDirection)
{
    DrawWall(target, wall);
    DrawPortals(target, frame.getList<Portal>(), playerDirection);
    DrawBoxes(target, frame.getList<Box>(), playerDirection);
    DrawGuys(target, frame.getList<Guy>(), playerDirection);
    DrawButtons(target, frame.getList<Button>(), playerDirection);
    DrawPlatforms(target, frame.getList<Platform>(), playerDirection);
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
                        32u*i,
                        32u*j,
                        32u*(i+1),
                        32u*(j+1),
                        Colour()));
            }
        }
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
        //if (box.getRelativeToPortal() == -1) {
            if (playerDirection == box.getTimeDirection()) {
                target.Draw(
                    Shape::Rectangle(
                        box.getX()/100,
                        box.getY()/100,
                        (box.getX()+ box.getSize())/100,
                        (box.getY()+box.getSize())/100,
                        Colour(255,0,255)));
            }
            else {
                int const x(box.getX()-box.getXspeed());
                int const y(box.getY()-box.getYspeed());
                target.Draw(
                    Shape::Rectangle(
                        x/100,
                        y/100,
                        (x + box.getSize())/100,
                        (y + box.getSize())/100,
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
        //if (guy.getRelativeToPortal() == -1) // if it is drawn when going through portal it may be somewhere strange, use same workaround as end of pause time flicker
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
                    left/100,
                    top/100,
                    right/100,
                    bottom/100,
                    pnc.colour));
            
            target.Draw(
                guy.getFacing() ?
                    Shape::Rectangle(
                        hmid/100,
                        top/100,
                        right/100,
                        vmid/100,
                        Colour(50,50,50)) :
                    Shape::Rectangle(
                        left/100,
                        top/100,
                        hmid/100,
                        vmid/100,
                        Colour(50,50,50)));
            
            if (guy.getBoxCarrying()) {
                target.Draw(
                    Shape::Rectangle(
                        (hmid - guy.getBoxCarrySize()/2)/100,
                        (top - guy.getBoxCarrySize())/100,
                        (hmid + guy.getBoxCarrySize()/2)/100,
                        top/100,
                        playerDirection == guy.getBoxCarryDirection() ?
                            Colour(150,0,150) :
                            Colour(0,150,0)));
            }
        }
    }
}

template<typename RandomAccessButtonRange>
void DrawButtons(
    RenderTarget& target,
    RandomAccessButtonRange const& buttonList,
    TimeDirection const playerDirection)
{
    foreach(Button const& button, buttonList)
    {
        Vector2<int> const pos(
            playerDirection == button.getTimeDirection() ?
                Vector2<int>(
                    button.getX(),
                    button.getY()) :
                Vector2<int>(
                    button.getX() - button.getXspeed(),
                    button.getY() - button.getYspeed()));

        target.Draw(
            Shape::Rectangle(
                pos.x/100,
                pos.y/100,
                (pos.x+button.getWidth())/100,
                (pos.y+button.getHeight())/100,
                button.getState() ?
                    Colour(150,255,150) :
                    Colour(255,150,150)));
    }
}

template<typename RandomAccessPlatformRange>
void DrawPlatforms(
    RenderTarget& target,
    RandomAccessPlatformRange const& platformList,
    TimeDirection const playerDirection)
{
    foreach (Platform const& platform, platformList) {
        PositionAndColour const pnc(
            playerDirection == platform.getTimeDirection() ?
                PositionAndColour(
                    platform.getX(),
                    platform.getY(),
                    Colour(50,0,0)) :
                PositionAndColour(
                    platform.getX() - platform.getXspeed(),
                    platform.getY() - platform.getYspeed(),
                    Colour(0,0,50)));

        target.Draw(
            Shape::Rectangle(
                pnc.x/100,
                pnc.y/100,
                (pnc.x+platform.getWidth())/100,
                (pnc.y+platform.getHeight())/100,
                pnc.colour));
    }
}

template<typename RandomAccessPortalRange>
void DrawPortals(
    RenderTarget& target,
    RandomAccessPortalRange const& portalList,
    TimeDirection const playerDirection)
{

    foreach(Portal const& portal, portalList)
    {
        PositionAndColour const pnc(
            playerDirection == portal.getTimeDirection() ?
                PositionAndColour(
                    portal.getX(),
                    portal.getY(),
                    Colour(120,120,120)) :
                PositionAndColour(
                    portal.getX()-portal.getXspeed(),
                    portal.getY()-portal.getYspeed(),
                    Colour(120,120,120)));

        target.Draw(
            Shape::Rectangle(
                pnc.x/100,
                pnc.y/100,
                (pnc.x+portal.getWidth())/100,
                (pnc.y+portal.getHeight())/100,
                pnc.colour));
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
                if (!pixelsWhichHaveBeenDrawnIn[static_cast<std::size_t>(getFrameNumber(frame)/10800.*640)]) {
                    target.Draw(
                        Shape::Rectangle(
                            static_cast<int>(getFrameNumber(frame)/10800.*640),
                            10,
                            static_cast<int>(getFrameNumber(frame)/10800.*640+1),
                            25,
                            Colour(250,0,0)));
                    pixelsWhichHaveBeenDrawnIn[static_cast<std::size_t>(getFrameNumber(frame)/10800.*640)] = true;
                }
            }
            else {
                assert(false && "I don't think that invalid frames can get updated");
            }
        }
    }
    if (playerFrame.isValidFrame()) {
        target.Draw(
            Shape::Rectangle(
                static_cast<int>(playerFrame.getFrameNumber()/10800.*640-1),
                10,
                static_cast<int>(playerFrame.getFrameNumber()/10800.*640+2),
                25,
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

Level MakeLevel(boost::multi_array<bool, 2> const& wall)
{
    ObjectList<Normal>  newObjectList;
    //newObjectList.add(Box(32400, 8000, 0, -600, 3200, -1, -1, FORWARDS));
    newObjectList.add(Box(46400, 14200, -1000, -500, 3200, -1, -1, FORWARDS));
    newObjectList.add(Box(46400, 10800, -1000, -500, 3200, -1, -1, FORWARDS));
    newObjectList.add(Box(46400, 17600, -1000, -500, 3200, -1, -1, FORWARDS));
    newObjectList.add(Box(46400, 21600, -500, -500, 3200, -1, -1, FORWARDS));
    newObjectList.add(Box(6400, 15600, 1000, -500, 3200, -1, -1, FORWARDS));
    newObjectList.add(Box(56400, 15600, 0, 0, 3200, -1, -1, FORWARDS));
    newObjectList.add(Guy(8700, 20000, 0, 0, 1600, 3200, 0, -1, false, 0, false, false, 0, INVALID, FORWARDS, 0));
    newObjectList.add(Button(30400, 44000, 0, 0, 3200, 800, 0, false, REVERSE));
    newObjectList.add(Platform(38400, 44800, 0, 0, 6400, 1600, 0, FORWARDS));
    newObjectList.add(Portal(20400, 30800, 0, 0, 4200, 4200, 0, FORWARDS, -1, true, 0, -16000, 0, 120, true, 0, true, false));
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
            ObjectList<Normal> (newObjectList),
            FrameID(0,UniverseID(10800)),
            TriggerSystem(
                1,
                0,
                std::vector<int>(),
                std::vector<PlatformDestinationPair>(
                    1,
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
                                20)))),
                AttachmentMap(
                    std::vector<Attachment>(1, Attachment(0,3200,-800)),
                    std::vector<Attachment>(1, Attachment(0,-4200,-3200)))));
}
}
