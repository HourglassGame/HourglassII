
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

namespace hg {
    void Draw(sf::RenderWindow& target, boost::shared_ptr<ObjectList>& frame, const std::vector<std::vector<bool> >& wall);
    void DrawWall(sf::RenderTarget& target, const std::vector<std::vector<bool> >& wallData);
    void DrawBoxes(sf::RenderTarget& target, const vector<boost::shared_ptr<Box> >& boxData);
    void DrawGuys(sf::RenderTarget& target, const vector<boost::shared_ptr<Guy> >& guyList);
    
    std::vector<std::vector<bool> > MakeWall();
    TimeEngine MakeTimeEngine(std::vector<std::vector<bool> >& wallData);
}

////////////////////////////////////////////////////////////
/// Entry point of application
///
/// \return Application exit code
///
////////////////////////////////////////////////////////////
int main()
{
    // Create main window
    sf::RenderWindow App(sf::VideoMode(640, 480), "Hourglass II");
    
    std::vector<std::vector<bool> > wall(hg::MakeWall());
    TimeEngine timeEngine(hg::MakeTimeEngine(wall));
    
    boost::posix_time::time_duration stepTime(0,0,0,boost::posix_time::time_duration::ticks_per_second()/60);
    
    // Start game loop
    while (App.IsOpened())
    {
        //Wait for step
        boost::posix_time::ptime startTime(boost::posix_time::microsec_clock::universal_time());
        
        //Load up input
        sf::Event event;
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
        
        hg::Input input(App.GetInput());
        
        //Get result from time-engine
        vector<boost::shared_ptr<ObjectList> > frame(timeEngine.getNextPlayerFrame(input.AsInputList()));
        if (frame.size() == 1) {
            //Draw result
            hg::Draw(App, frame[0], wall);
            
            //Wait for next step
            while (boost::posix_time::microsec_clock::universal_time() - startTime < stepTime) {
                boost::this_thread::sleep(boost::posix_time::milliseconds(1));
            }
        }
        else {
            //PARADOX FOO
            return EXIT_FAILURE;
        }
    }
    
    return EXIT_SUCCESS;
}

void hg::Draw(sf::RenderWindow& target, boost::shared_ptr<ObjectList>& frame, const std::vector<std::vector<bool> >& wallData)
{
    hg::DrawWall(target, wallData);
    hg::DrawBoxes(target, frame->getBoxList());
    hg::DrawGuys(target, frame->getGuyList());
    target.Display();
}

void hg::DrawWall(sf::RenderTarget& target, const std::vector<std::vector<bool> >& wall)
{
    target.Clear(sf::Color(255,255,255));
    for(unsigned int i = 0; i < wall.size(); ++i) {
        for(unsigned int j = 0; j < wall.at(i).size(); ++j) {
            if (wall.at(i).at(j)) {
                target.Draw(sf::Shape::Rectangle(32*i, 32*j, 32*(i+1), 32*(j+1), sf::Color()));
            }
        }
    }
}

void hg::DrawBoxes(sf::RenderTarget& target, const vector<boost::shared_ptr<Box> >& boxList)
{
    foreach(const boost::shared_ptr<Box>& box, boxList) {
        target.Draw(sf::Shape::Rectangle(box->getX()/100,
                                         box->getY()/100,
                                        (box->getX()+ box->getSize())/100,
                                         (box->getY()+box->getSize())/100,
                                         sf::Color(150,150,0)));
    }
}

void hg::DrawGuys(sf::RenderTarget& target, const vector<boost::shared_ptr<Guy> >& guyList)
{
    foreach(const boost::shared_ptr<Guy>& guy, guyList) {
        target.Draw(sf::Shape::Rectangle(guy->getX()/100,
                                         guy->getY()/100,
                                         (guy->getX()+ guy->getWidth())/100,
                                         (guy->getY()+guy->getHeight())/100,
                                         sf::Color(150,150,0)));
    }
}


std::vector<std::vector<bool> > hg::MakeWall()
{
    using namespace boost::assign;
    std::vector<std::vector<bool> > wall;
    std::vector<bool> row;
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
    std::vector<std::vector<bool> > actualWall;
    actualWall.resize(wall.at(0).size());
    foreach(std::vector<bool>& column, actualWall) {
        column.resize(wall.size());
    }
    for(unsigned int i = 0; i < wall.size(); ++i) {
        for(unsigned int j = 0; j < wall.at(i).size(); ++j) {
            actualWall.at(j).at(i) = wall.at(i).at(j);
        }
    }
    //Logger::GetLogger().Log("Made Wall", loglevel::FINE);
    return actualWall;
}

TimeEngine hg::MakeTimeEngine(std::vector<std::vector<bool> >& wall)
{
    TimeEngine newEngine(5400,wall,3200,50);
    boost::shared_ptr<ObjectList> newObjectList(new ObjectList());
    newObjectList->addBox(6400, 25600, 0 ,0, 3200, 1);
    //int x, int y, int xspeed, int yspeed, int width, int height, int timeDirection, bool boxCarrying, int boxCarrySize, int boxCarryDirection, int relativeIndex, int subimage
    newObjectList->addGuy(22000, 6400, 0, 0, 1600, 3200, 1, false, 0, 0, 0, 0);
    std::cout << newEngine.checkConstistencyAndPropagateLevel(newObjectList,0) << std::endl;
    return newEngine;
}

