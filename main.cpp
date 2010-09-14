////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ObjectList.h"
#include "TimeEngine.h"
#include "Hg_Input.h"

#include <SFML/Graphics.hpp>

#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/assign.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

#include <sstream>
#include <iostream>

#define foreach BOOST_FOREACH
using namespace ::hg;
using namespace ::std;
using namespace ::sf;
using namespace ::boost;
namespace {
    void Draw(RenderWindow& target, const ObjectList& frame, const vector<vector<bool> >& wall, TimeDirection& playerDirection);
    void DrawTimeline(RenderTarget& target, TimeEngine::FrameListList& waves, NewFrameID& playerFrame);
    void DrawWall(RenderTarget& target, const vector<vector<bool> >& wallData);
    void DrawBoxes(RenderTarget& target, const vector<Box>& boxData, TimeDirection&);
    void DrawGuys(RenderTarget& target, const vector<Guy>& guyList, TimeDirection&);

    vector<vector<bool> > MakeWall();
    TimeEngine MakeTimeEngine(vector<vector<bool> >& wallData);
}

////////////////////////////////////////////////////////////
/// Entry point of application
///
/// \return Application exit code
///
////////////////////////////////////////////////////////////
int main()
{
    RenderWindow App(VideoMode(640, 480), "Hourglass II");

    vector<vector<bool> > wall(MakeWall());
    TimeEngine timeEngine(MakeTimeEngine(wall));

    float stepTime(1.f/60);
    ::hg::Input input;
    while (App.IsOpened())
    {
        Clock clock;

        Event event;
        while (App.GetEvent(event))
        {
            switch (event.Type) {
                case sf::Event::Closed:
                    App.Close();
                    break;
                default:
                    break;
            }
        }

        input.updateState(App.GetInput());
        //cout << "called from main" << endl;
        tuple<NewFrameID, TimeEngine::FrameListList, TimeDirection> waveInfo(timeEngine.runToNextPlayerFrame(input.AsInputList()));
        Draw(App, timeEngine.getPostPhysics(waveInfo.get<0>()), wall, waveInfo.get<2>());
        DrawTimeline(App, waveInfo.get<1>(), waveInfo.get<0>());


        while (clock.GetElapsedTime() < stepTime) {
            this_thread::sleep(posix_time::milliseconds(1));
        }
        {
            stringstream fpsstring;
            fpsstring << (1./clock.GetElapsedTime());
            sf::String fpsglyph(fpsstring.str());
            fpsglyph.SetPosition(600, 465);
            fpsglyph.SetSize(8.f);
            App.Draw(fpsglyph);
        }
        App.Display();
    }
    return EXIT_SUCCESS;
}

namespace  {
void Draw(RenderWindow& target, const ObjectList& frame, const vector<vector<bool> >& wallData, TimeDirection& playerDirection)
{
    DrawWall(target, wallData);
    DrawBoxes(target, frame.getBoxListRef(), playerDirection);
    DrawGuys(target, frame.getGuyListRef(), playerDirection);
}

void DrawWall(sf::RenderTarget& target, const std::vector<std::vector<bool> >& wall)
{
    target.Clear(Color(255,255,255));
    for(unsigned int i = 0; i < wall.size(); ++i) {
        for(unsigned int j = 0; j < wall.at(i).size(); ++j) {
            if (wall.at(i).at(j)) {
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

void DrawBoxes(RenderTarget& target, const vector<Box>& boxList, TimeDirection& playerDirection)
{
    foreach(const Box& box, boxList) {
        if (playerDirection == box.getTimeDirection())
        {
             target.Draw(Shape::Rectangle(box.getX()/100,
                                         box.getY()/100,
                                        (box.getX()+ box.getSize())/100,
                                         (box.getY()+box.getSize())/100,
                                         Color(255,0,255)));
        }
        else
        {
            int x = box.getX()-box.getXspeed();
            int y = box.getY()-box.getYspeed();
            target.Draw(Shape::Rectangle(x/100,
                                         y/100,
                                        (x+ box.getSize())/100,
                                         (y+box.getSize())/100,
                                         Color(255,0,255)));
        }

    }
}

void DrawGuys(RenderTarget& target, const vector<Guy>& guyList, TimeDirection& playerDirection)
{
    foreach(const Guy& guy, guyList) {
         if (playerDirection == guy.getTimeDirection())
         {
            target.Draw(Shape::Rectangle(guy.getX()/100,
                                         guy.getY()/100,
                                         (guy.getX()+ guy.getWidth())/100,
                                         (guy.getY()+guy.getHeight())/100,
                                         Color(150,150,0)));
            if (guy.getBoxCarrying())
            {
                target.Draw(Shape::Rectangle((guy.getX() + guy.getWidth()/2 - guy.getBoxCarrySize()/2)/100,
                                             (guy.getY() - guy.getBoxCarrySize())/100,
                                             (guy.getX() + guy.getWidth()/2 + guy.getBoxCarrySize()/2)/100,
                                             (guy.getY())/100,
                                             Color(0,0,255)));
            }
        }
        else
        {
            int x = guy.getX()-guy.getXspeed();
            int y = guy.getY()-guy.getYspeed();
            target.Draw(Shape::Rectangle(x/100,
                                         y/100,
                                         (x+ guy.getWidth())/100,
                                         (y+guy.getHeight())/100,
                                         Color(150,150,0)));
            if (guy.getBoxCarrying())
            {
                target.Draw(Shape::Rectangle((x + guy.getWidth()/2 - guy.getBoxCarrySize()/2)/100,
                                             (y - guy.getBoxCarrySize())/100,
                                             (x + guy.getWidth()/2 + guy.getBoxCarrySize()/2)/100,
                                             y/100,
                                             Color(0,0,255)));
            }
        }
    }
}

void DrawTimeline(RenderTarget& target, TimeEngine::FrameListList& waves, NewFrameID& playerFrame)
{
    bool pixelsWhichHaveBeenDrawnIn[640] = {false};
    foreach(const FrameUpdateSet& lists, waves) {
        foreach (NewFrameID frame, lists) {
        if (frame.isValidFrame()) {
            if (!pixelsWhichHaveBeenDrawnIn[static_cast<int> ((frame.frame()/10800.f)*640)]) {
                target.Draw(Shape::Rectangle((frame.frame()/10800.f)*640,
                                             10,
                                             (frame.frame()/10800.f)*640+1,
                                             25,
                                             Color(250,0,0)));
                pixelsWhichHaveBeenDrawnIn[static_cast<int> ((frame.frame()/10800.f)*640)] = true;
            }
        }
        else {
            assert(false && "I don't think that invalid frames can get updated");
        }
        }
    }
    assert(playerFrame.isValidFrame());

    target.Draw(Shape::Rectangle((playerFrame.frame()/10800.f)*640-1,
                                             10,
                                             (playerFrame.frame()/10800.f)*640+2,
                                             25,
                                             Color(200,200,0)));
}

vector<vector<bool> > MakeWall()
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
    vector<vector<bool> > actualWall;
    actualWall.resize(wall.at(0).size());
    foreach(vector<bool>& column, actualWall) {
        column.resize(wall.size());
    }
    for(unsigned int i = 0; i < wall.size(); ++i) {
        for(unsigned int j = 0; j < wall.at(i).size(); ++j) {
            actualWall.at(j).at(i) = wall.at(i).at(j);
        }
    }
    return actualWall;
}

TimeEngine MakeTimeEngine(vector<vector<bool> >& wall)
{
    MutableObjectList newObjectList;
    newObjectList.addBox(Box(46400, 15600, -1000, -500, 3200, FORWARDS));
    newObjectList.addBox(Box(6400, 15600, 1000, -500, 3200, FORWARDS));
    newObjectList.addGuy(Guy(8700, 20000, 0, 0, 1600, 3200, false, false, 0, PAUSE, FORWARDS, 0, 0));
    return TimeEngine(10800,wall,3200,50,ObjectList(newObjectList),NewFrameID(0,10800));
}
}
