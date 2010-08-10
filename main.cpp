
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
#define foreach BOOST_FOREACH
using namespace ::hg;
using namespace ::std;
using namespace ::sf;
namespace hg {
    void Draw(RenderWindow& target, const ObjectList& frame, const vector<vector<bool> >& wall);
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
    
    ::boost::posix_time::time_duration stepTime(0,0,0,boost::posix_time::time_duration::ticks_per_second()/60);
    
    while (App.IsOpened())
    {
        ::boost::posix_time::ptime startTime(boost::posix_time::microsec_clock::universal_time());
        
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
        const ::hg::Input input(App.GetInput());
        
        cout << "called from main" << endl;
        ObjectList frame(timeEngine.getNextPlayerFrame(input.AsInputList()));
        
        Draw(App, frame, wall);

        while (::boost::posix_time::microsec_clock::universal_time() - startTime < stepTime) {
            ::boost::this_thread::sleep(boost::posix_time::milliseconds(1));
        }

    }
    
    return EXIT_SUCCESS;
}

void ::hg::Draw(RenderWindow& target, const ObjectList& frame, const vector<vector<bool> >& wallData)
{
    DrawWall(target, wallData);
    DrawBoxes(target, frame.getBoxListRef());
    DrawGuys(target, frame.getGuyListRef());
    target.Display();
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
                        32*static_cast<unsigned int>(i), 
                        32*static_cast<unsigned int>(j),
                        32*(static_cast<unsigned int>(i)+1),
                        32*(static_cast<unsigned int>(j)+1),
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
                                         Color(150,150,0)));
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
    return TimeEngine(5400,wall,3200,50,newObjectList,0);
}

