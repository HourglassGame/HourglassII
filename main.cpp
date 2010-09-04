
////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

#include <SFML/Graphics.hpp>
#include <boost/thread.hpp>
#include "TimeEngine.h"
#include "Hg_Input.h"
#include <iostream>
#include <boost/assign.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#define foreach BOOST_FOREACH
using namespace ::hg;
using namespace ::std;
using namespace ::sf;
using namespace ::boost;
namespace hg {
    void Draw(RenderWindow& target, const ObjectList& frame, const vector<vector<bool> >& wall);
    void DrawTimeline(RenderTarget& target, TimeEngine::FrameListList& waves);
    void DrawWall(RenderTarget& target, const vector<vector<bool> >& wallData);
    void DrawBoxes(RenderTarget& target, const vector<Box>& boxData);
    void DrawGuys(RenderTarget& target, const vector<Guy>& guyList);

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

    posix_time::time_duration stepTime(0,0,0,posix_time::time_duration::ticks_per_second()/60);
    ::hg::Input input;
    while (App.IsOpened())
    {
        posix_time::ptime startTime(posix_time::microsec_clock::universal_time());

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
        tuple<FrameID, TimeEngine::FrameListList> waveInfo(timeEngine.runToNextPlayerFrame(input.AsInputList()));
        Draw(App, timeEngine.getPostPhysics(waveInfo.get<0>()), wall);
        DrawTimeline(App, waveInfo.get<1>());
        App.Display();

        while (posix_time::microsec_clock::universal_time() - startTime < stepTime) {
            this_thread::sleep(posix_time::milliseconds(1));
        }
    }
    return EXIT_SUCCESS;
}

void ::hg::Draw(RenderWindow& target, const ObjectList& frame, const vector<vector<bool> >& wallData)
{
    DrawWall(target, wallData);
    DrawBoxes(target, frame.getBoxListRef());
    DrawGuys(target, frame.getGuyListRef());
}

void ::hg::DrawWall(sf::RenderTarget& target, const std::vector<std::vector<bool> >& wall)
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

void ::hg::DrawBoxes(RenderTarget& target, const vector<Box>& boxList)
{
    foreach(const Box& box, boxList) {
        target.Draw(Shape::Rectangle(box.getX()/100,
                                         box.getY()/100,
                                        (box.getX()+ box.getSize())/100,
                                         (box.getY()+box.getSize())/100,
                                         Color(255,0,255)));
    }
}

void ::hg::DrawGuys(RenderTarget& target, const vector<Guy>& guyList)
{
    foreach(const Guy& guy, guyList) {
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
}

void ::hg::DrawTimeline(RenderTarget& target, TimeEngine::FrameListList& waves)
{

    foreach(const FrameUpdateSet& lists, waves) {
        foreach (FrameID frame, lists) {
            target.Draw(Shape::Rectangle((frame/10800.f)*640,
                                         10,
                                         (frame/10800.f)*640+1,
                                         25,
                                         Color(250,0,0)));
        }
    }
}

vector<vector<bool> > hg::MakeWall()
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

TimeEngine hg::MakeTimeEngine(vector<vector<bool> >& wall)
{
    ObjectList newObjectList;
    newObjectList.addBox(Box(6400, 25600, 0 ,0, 3200, FORWARDS));
    newObjectList.addGuy(Guy(22000, 6400, 0, 0, 1600, 3200, false, 0, PAUSE, FORWARDS, 0, 0));
    return TimeEngine(10800,wall,3200,50,newObjectList,0);
}
