////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "TimeEngine.h"
#include "PlayerVictoryException.h"
#include "Hg_Input.h"
#include "Level.h"
#include <SFML/Graphics.hpp>

#include <boost/multi_array.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/assign.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/nvp.hpp>

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

#include <cmath>
#include "nedmalloc.c"
#include <new>

void* operator new(size_t size) throw(std::bad_alloc)
{
    while (true) {
        void* pointer(nedalloc::nedmalloc(size));
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
    nedalloc::nedfree(p);
}

void* operator new(size_t size, const std::nothrow_t &) throw()
{
    while (true) {
        void* pointer(nedalloc::nedmalloc(size));
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

void operator delete(void *p, const std::nothrow_t &) throw()
{
    nedalloc::nedfree(p);
}

void* operator new[](size_t size) throw(std::bad_alloc)
{
    return ::operator new(size);
}

void operator delete[](void *p) throw()
{
    ::operator delete(p);
}

void* operator new[](size_t size, const std::nothrow_t & nothrow) throw()
{
   return ::operator new(size, nothrow);
}

void operator delete[](void *p, const std::nothrow_t & nothrow) throw()
{
    ::operator delete(p, nothrow);
}

#define foreach BOOST_FOREACH
using namespace ::hg;
using namespace ::std;
using namespace ::sf;
using namespace ::boost;
namespace {
    void Draw(RenderWindow& target, const ObjectPtrList& frame, const ::boost::multi_array<bool, 2>& wall, TimeDirection& playerDirection);
    void DrawTimeline(RenderTarget& target, TimeEngine::FrameListList& waves, Frame* playerFrame);
    void DrawWall(RenderTarget& target, const ::boost::multi_array<bool, 2>& wallData);
    void DrawBoxes(RenderTarget& target, const vector<const Box*>& boxData, TimeDirection&);
    void DrawGuys(RenderTarget& target, const vector<const Guy*>& guyList, TimeDirection&);
    void DrawButtons(RenderTarget& target, const vector<const Button*>& buttonList, TimeDirection& playerDirection);
    void DrawPlatforms(RenderTarget& target, const vector<const Platform*>& platformList, TimeDirection& playerDirection);
    void DrawPortals(RenderTarget& target, const vector<const Portal*>& portalList, TimeDirection& playerDirection);

    ::boost::multi_array<bool, 2> MakeWall();
    Level MakeLevel(const ::boost::multi_array<bool, 2>& wallData);
}

////////////////////////////////////////////////////////////
/// Entry point of application
///
/// \return Application exit code
///
////////////////////////////////////////////////////////////
int main()
{
{
    RenderWindow app(VideoMode(640, 480), "Hourglass II");
    app.UseVerticalSync(true);
    app.SetFramerateLimit(60);
    ::boost::multi_array<bool, 2> wall(MakeWall());
    TimeEngine timeEngine(MakeLevel(wall));

    ::hg::Input input;
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
        if (app.GetInput().IsKeyDown(sf::Key::S)) {
            const vector<InputList> replay(timeEngine.getReplayData());
            std::ofstream ofs("replay");
            {
                boost::archive::binary_oarchive out(ofs);
                out << BOOST_SERIALIZATION_NVP(replay);
            }
        }
        input.updateState(app.GetInput());
        //cout << "called from main" << endl;
        try{
            TimeEngine::RunResult waveInfo;
            timeEngine.runToNextPlayerFrame(input.AsInputList()).swap(waveInfo);
            if (waveInfo.currentPlayerFrame) {
                Draw
                (
                    app,
                    waveInfo.currentPlayerFrame->getPostPhysics(),
                    wall,
                    waveInfo.currentPlayerDirection
                );
            }
            else {
                Draw(app, timeEngine.getFrame(FrameID(abs((app.GetInput().GetMouseX()*10800/640)%10800),10800))->getPostPhysics(), wall, waveInfo.currentPlayerDirection);
            }
            DrawTimeline(app, waveInfo.updatedFrames, waveInfo.currentPlayerFrame);
        }
        catch (hg::PlayerVictoryException& playerWon) {
            cout << "Congratulations, a winner is you!\n";
            return EXIT_SUCCESS;
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
    }
    #if 0
    {

    ::boost::multi_array<bool, 2> wall(MakeWall());
    TimeEngine timeEngine(MakeLevel(wall));

    vector<InputList> input;
    {
        // create and open an archive for input
        std::ifstream ifs("replay");
        boost::archive::binary_iarchive ia(ifs);
        // read class state from archive
        ia >> BOOST_SERIALIZATION_NVP(input);
        // archive and stream closed when destructors are called
    }
    vector<double> fpses;
    fpses.reserve(input.size());
    RenderWindow app(VideoMode(640, 480), "Hourglass II");
    app.UseVerticalSync(true);
    app.SetFramerateLimit(60);
    foreach(const InputList& inputpart, input)
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
        //cout << "called from main" << endl;
        try{
            tuple<FrameID, FrameID, TimeEngine::FrameListList, TimeDirection> waveInfo(timeEngine.runToNextPlayerFrame(inputpart));
            if (waveInfo.get<0>().isValidFrame()) {
                Draw
                (
                    app,
                    timeEngine.getPostPhysics
                    (
                        waveInfo.get<0>(),
                        waveInfo.get<1>().universe().pauseDepth() == waveInfo.get<0>().universe().pauseDepth()
                            ?
                            PauseInitiatorID(pauseinitiatortype::INVALID,0,0)
                            :
                            waveInfo.get<1>().universe().initiatorID()
                    ),
                    wall,
                    waveInfo.get<3>()
                );
            }
            else {
                Draw(app, timeEngine.getPostPhysics(FrameID(abs((app.GetInput().GetMouseX()*10800/640)%10800),10800),PauseInitiatorID(pauseinitiatortype::INVALID,0,0)), wall, waveInfo.get<3>());
            }
            DrawTimeline(app, waveInfo.get<2>(), waveInfo.get<0>());
        }
        catch (hg::PlayerVictoryException& playerWon) {
            cout << "Congratulations, a winner is you!\n";
            return EXIT_SUCCESS;
        }

        if (app.GetFrameTime() != 0) {
            stringstream fpsstring;
            double fps(1./app.GetFrameTime());
            cout << fps << "\n";
            fpses.push_back(fps);
            fpsstring << fps;
            sf::String fpsglyph(fpsstring.str());
            fpsglyph.SetPosition(600, 465);
            fpsglyph.SetSize(8.f);
            app.Draw(fpsglyph);
        }
        app.Display();
    }
    double mean(0);
    foreach(double fps, fpses) {
        mean += fps;
    }
    mean /= fpses.size();
    cout << "mean fps: " << mean << "\n";
    cout << "best fps: " << *max_element(fpses.begin(), fpses.end()) << "\n";
    cout << "worst fps: " << *min_element(fpses.begin(), fpses.end()) << "\n";
    }
    #endif
    return EXIT_SUCCESS;
}

namespace  {
void Draw(RenderWindow& target, const ObjectPtrList& frame, const ::boost::multi_array<bool, 2>& wall, TimeDirection& playerDirection)
{
    DrawWall(target, wall);
    DrawPortals(target, frame.getPortalListRef(), playerDirection);
    DrawBoxes(target, frame.getBoxListRef(), playerDirection);
    DrawGuys(target, frame.getGuyListRef(), playerDirection);
    DrawButtons(target, frame.getButtonListRef(), playerDirection);
    DrawPlatforms(target, frame.getPlatformListRef(), playerDirection);

}

void DrawWall(sf::RenderTarget& target, const ::boost::multi_array<bool, 2>& wall)
{
    target.Clear(Color(255,255,255));
    for(unsigned int i = 0; i < wall.shape()[0]; ++i) {
        for(unsigned int j = 0; j < wall.shape()[1]; ++j) {
            if (wall[i][j]) {
                target.Draw
                (
                    Shape::Rectangle
                    (
                        32u*i,
                        32u*j,
                        32u*(i+1),
                        32u*(j+1),
                        Color()
                    )
                );
            }
        }
    }
}

void DrawBoxes(RenderTarget& target, const vector<const Box*>& boxList, TimeDirection& playerDirection)
{
    foreach(const Box* box, boxList) {
        if (playerDirection == box->getTimeDirection())
        {
             target.Draw(Shape::Rectangle(
                box->getX()/100,
                box->getY()/100,
                (box->getX()+ box->getSize())/100,
                (box->getY()+box->getSize())/100,
                Color(255,0,255))
            );
        }
        else
        {
            int x = box->getX()-box->getXspeed();
            int y = box->getY()-box->getYspeed();
            target.Draw(Shape::Rectangle(
                x/100,
                y/100,
                (x+ box->getSize())/100,
                (y+box->getSize())/100,
                Color(0,255,0))
            );
        }
    }
}

void DrawGuys(RenderTarget& target, const vector<const Guy*>& guyList, TimeDirection& playerDirection)
{
    foreach(const Guy* guy, guyList) {
        if (guy->getRelativeToPortal() == -1) // if it is drawn when going through portal it may be somewhere strange, use same workaround as end of pause time flicker
        {
            int x,y;
            Color guyColor;
            if (playerDirection == guy->getTimeDirection())
            {
                x = guy->getX();
                y = guy->getY();
                guyColor = Color(150,150,0);
            }
            else
            {
                x = guy->getX()-guy->getXspeed();
                y = guy->getY()-guy->getYspeed();
                guyColor = Color(0,0,150);
            }

            target.Draw(Shape::Rectangle(
                x/100,
                y/100,
                (x+ guy->getWidth())/100,
                (y+guy->getHeight())/100,
                guyColor)
            );

            if (guy->getBoxCarrying())
            {
                Color boxColor;
                if (playerDirection == guy->getBoxCarryDirection())
                {
                    boxColor = Color(150,0,150);
                }
                else
                {
                    boxColor = Color(0,150,0);
                }

                target.Draw(Shape::Rectangle(
                    (x + guy->getWidth()/2 - guy->getBoxCarrySize()/2)/100,
                    (y - guy->getBoxCarrySize())/100,
                    (x + guy->getWidth()/2 + guy->getBoxCarrySize()/2)/100,
                    y/100,
                    boxColor)
                );
            }
        }
    }
}

void DrawButtons(RenderTarget& target, const vector<const Button*>& buttonList, TimeDirection& playerDirection)
{
     foreach(const Button* button, buttonList)
     {
        Color buttonColor;
        if (button->getState())
        {
            buttonColor = Color(150,255,150);
        }
        else
        {
            buttonColor = Color(255,150,150);
        }

        int x,y;
        if (playerDirection == button->getTimeDirection())
        {
            x = button->getX();
            y = button->getY();
        }
        else
        {
            x = button->getX()-button->getXspeed();
            y = button->getY()-button->getYspeed();
        }

        target.Draw(Shape::Rectangle(
            x/100,
            y/100,
            (x+3200)/100,
            (y+800)/100,
            buttonColor)
        );
     }
}

void DrawPlatforms(RenderTarget& target, const vector<const Platform*>& platformList, TimeDirection& playerDirection)
{

     foreach(const Platform* platform, platformList)
     {
        int x,y;

        Color platformColor;
        if (playerDirection == platform->getTimeDirection())
        {
            x = platform->getX();
            y = platform->getY();
            platformColor = Color(50,0,0);
        }
        else
        {
            x = platform->getX()-platform->getXspeed();
            y = platform->getY()-platform->getYspeed();
            platformColor = Color(0,0,50);
        }
        //cout << x << " " << y << " " << platform.getXspeed() << " " << platform.getYspeed() << endl;

        target.Draw(Shape::Rectangle(
            x/100,
            y/100,
            (x+platform->getWidth())/100,
            (y+platform->getHeight())/100,
            platformColor)
        );
     }
}

void DrawPortals(RenderTarget& target, const vector<const Portal*>& portalList, TimeDirection& playerDirection)
{

     foreach(const Portal* portal, portalList)
     {
        int x,y;

        Color portalColor;
        if (playerDirection == portal->getTimeDirection())
        {
            x = portal->getX();
            y = portal->getY();
            portalColor = Color(120,120,120);
        }
        else
        {
            x = portal->getX()-portal->getXspeed();
            y = portal->getY()-portal->getYspeed();
            portalColor = Color(120,120,120);
        }

        target.Draw(Shape::Rectangle(
            x/100,
            y/100,
            (x+portal->getWidth())/100,
            (y+portal->getHeight())/100,
            portalColor)
        );
     }
}

void DrawTimeline(RenderTarget& target, TimeEngine::FrameListList& waves, Frame* playerFrame)
{
    bool pixelsWhichHaveBeenDrawnIn[640] = {false};
    foreach(const FrameUpdateSet& lists, waves) {
        foreach (Frame* frame, lists) {
            if (frame) {

                if (!pixelsWhichHaveBeenDrawnIn[static_cast<unsigned int> ((frame->getFrameNumber()/10800.f)*640)]) {
                    target.Draw(Shape::Rectangle((frame->getFrameNumber()/10800.f)*640,
                                                10,
                                                (frame->getFrameNumber()/10800.f)*640+1,
                                                25,
                                                Color(250,0,0)));
                    pixelsWhichHaveBeenDrawnIn[static_cast<int> ((frame->getFrameNumber()/10800.f)*640)] = true;
                }
            }
            else {
                assert(false && "I don't think that invalid frames can get updated");
            }
        }
    }
    if(playerFrame) {
        target.Draw(Shape::Rectangle((playerFrame->getFrameNumber()/10800.f)*640-1,
                                             10,
                                             (playerFrame->getFrameNumber()/10800.f)*640+2,
                                             25,
                                             Color(200,200,0)));
    }
     target.Draw(Shape::Rectangle((3000.f/10800.f)*640-1,
                                             10,
                                             (3000.f/10800.f)*640+2,
                                             25,
                                             Color(0,255,0)));
}

::boost::multi_array<bool, 2> MakeWall()
{
    using namespace ::boost::assign;
    vector<vector<bool> > wall;
    vector<bool> row;
    row += 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1;
    wall.push_back(row);
    row.clear();
    row += 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1;
    wall.push_back(row);
    row.clear();
    row += 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1;
    wall.push_back(row);
    row.clear();
    row += 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1;
    wall.push_back(row);
    row.clear();
    row += 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1;
    wall.push_back(row);
    row.clear();
    row += 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1;
    wall.push_back(row);
    row.clear();
    row += 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1;
    wall.push_back(row);
    row.clear();
    row += 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1;
    wall.push_back(row);
    row.clear();
    row += 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1;
    wall.push_back(row);
    row.clear();
    row += 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1;
    wall.push_back(row);
    row.clear();
    row += 1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1;
    wall.push_back(row);
    row.clear();
    row += 1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1;
    wall.push_back(row);
    row.clear();
    row += 1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1;
    wall.push_back(row);
    row.clear();
    row += 1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1;
    wall.push_back(row);
    row.clear();
    row += 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1;
    wall.push_back(row);
    row.clear();
    boost::multi_array<bool, 2> actualWall(boost::extents[wall.at(0).size()][wall.size()]);
    for(unsigned int i = 0; i < wall.size(); ++i) {
        for(unsigned int j = 0; j < wall.at(i).size(); ++j) {
            actualWall[j][i] = wall.at(i).at(j);
        }
    }
    return actualWall;
}

Level MakeLevel(const ::boost::multi_array<bool, 2>& wall)
{
    ObjectList newObjectList;
    newObjectList.add(Box(32400, 10000, 0, 0, 3200, FORWARDS, 0));
    //newObjectList.add(Box(46400, 15600, -1000, -500, 3200, FORWARDS, 0));
    //newObjectList.add(Box(6400, 15600, 1000, -500, 3200, FORWARDS, 0));
    //newObjectList.add(Box(56400, 15600, 0, 0, 3200, FORWARDS, 0));
    newObjectList.add(Guy(8700, 20000, 0, 0, 1600, 3200, -1, false, false, 0, INVALID, 0, FORWARDS, 0, 0));
    newObjectList.add(Button(30400, 44000, 0, 0, 0, false, REVERSE, 0));
    newObjectList.add(Platform(38400, 44800, 0, 0, 6400, 1600, 0, FORWARDS, 0));
    newObjectList.add(Portal(20400, 30800, 0, 0, 4200, 4200, 0, FORWARDS, 0, -1, true, 0, 0, 0, 4000, false));

    return 
        Level(
        3,
        10800,
        wall,
        3200,
        30,
        ObjectList(newObjectList),
        FrameID(0,10800),
        AttachmentMap
        (
            ::std::vector< ::boost::tuple<int, int, int> >(1,::boost::tuple<int, int, int>(0,3200,-800)),
            ::std::vector< ::boost::tuple<int, int, int> >(1,::boost::tuple<int, int, int>(0,-4200,-3200))
        ),
        TriggerSystem
        (
            ::std::vector<int>(1, 0),
            1,
            1,
            1,
            ::std::vector<PlatformDestination>
            (
                1,
                PlatformDestination
                (
                    38400,
                    0,
                    0,
                    0,
                    32000,
                    300,
                    50,
                    50
                )
            ),
            ::std::vector<PlatformDestination>
            (
                1,
                PlatformDestination
                (
                    38400,
                    0,
                    0,
                    0,
                    43800,
                    300,
                    20,
                    20
                )
            )
        )
    );
}
}
