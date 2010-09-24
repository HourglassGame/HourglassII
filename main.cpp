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

#include <cmath>

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
    void DrawButtons(RenderTarget& target, const vector<Button>& buttonList, TimeDirection& playerDirection);
    void DrawPlatforms(RenderTarget& target, const vector<Platform>& platformList, TimeDirection& playerDirection);

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
    RenderWindow app(VideoMode(640, 480), "Hourglass II");
    app.UseVerticalSync(true);
    app.SetFramerateLimit(60);
    vector<vector<bool> > wall(MakeWall());
    TimeEngine timeEngine(MakeTimeEngine(wall));

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

        input.updateState(app.GetInput());
        //cout << "called from main" << endl;
        tuple<NewFrameID, TimeEngine::FrameListList, TimeDirection> waveInfo(timeEngine.runToNextPlayerFrame(input.AsInputList()));
        
        
        Draw(app, timeEngine.getPostPhysics(waveInfo.get<0>().isValidFrame()?waveInfo.get<0>():NewFrameID(abs((app.GetInput().GetMouseX()*10800/640)%10800),10800)), wall, waveInfo.get<2>());
        DrawTimeline(app, waveInfo.get<1>(), waveInfo.get<0>());

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
void Draw(RenderWindow& target, const ObjectList& frame, const vector<vector<bool> >& wallData, TimeDirection& playerDirection)
{
    DrawWall(target, wallData);
    DrawBoxes(target, frame.getBoxListRef(), playerDirection);
    DrawGuys(target, frame.getGuyListRef(), playerDirection);
    DrawButtons(target, frame.getButtonListRef(), playerDirection);
    DrawPlatforms(target, frame.getPlatformListRef(), playerDirection);
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
             target.Draw(Shape::Rectangle(
                box.getX()/100,
                box.getY()/100,
                (box.getX()+ box.getSize())/100,
                (box.getY()+box.getSize())/100,
                Color(255,0,255))
            );
        }
        else
        {
            int x = box.getX()-box.getXspeed();
            int y = box.getY()-box.getYspeed();
            target.Draw(Shape::Rectangle(
                x/100,
                y/100,
                (x+ box.getSize())/100,
                (y+box.getSize())/100,
                Color(0,255,0))
            );
        }

    }
}

void DrawGuys(RenderTarget& target, const vector<Guy>& guyList, TimeDirection& playerDirection)
{
    foreach(const Guy& guy, guyList) {
        int x,y;
        Color guyColor;
        if (playerDirection == guy.getTimeDirection())
        {
            x = guy.getX();
            y = guy.getY();
            guyColor = Color(150,150,0);
        }
        else
        {
            x = guy.getX()-guy.getXspeed();
            y = guy.getY()-guy.getYspeed();
            guyColor = Color(0,0,150);
        }

        target.Draw(Shape::Rectangle(
            x/100,
            y/100,
            (x+ guy.getWidth())/100,
            (y+guy.getHeight())/100,
            guyColor)
        );

        if (guy.getBoxCarrying())
        {
            Color boxColor;
            if (playerDirection == guy.getBoxCarryDirection())
            {
                boxColor = Color(150,0,150);
            }
            else
            {
                boxColor = Color(0,150,0);
            }

            target.Draw(Shape::Rectangle(
                (x + guy.getWidth()/2 - guy.getBoxCarrySize()/2)/100,
                (y - guy.getBoxCarrySize())/100,
                (x + guy.getWidth()/2 + guy.getBoxCarrySize()/2)/100,
                y/100,
                boxColor)
            );
        }
    }
}

void DrawButtons(RenderTarget& target, const vector<Button>& buttonList, TimeDirection& playerDirection)
{
     foreach(const Button& button, buttonList)
     {
        Color buttonColor;
        if (button.getState())
        {
            buttonColor = Color(150,255,150);
        }
        else
        {
            buttonColor = Color(255,150,150);
        }

        int x,y;
        if (playerDirection == button.getTimeDirection())
        {
            x = button.getX();
            y = button.getY();
        }
        else
        {
            x = button.getX()-button.getXspeed();
            y = button.getY()-button.getYspeed();
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

void DrawPlatforms(RenderTarget& target, const vector<Platform>& platformList, TimeDirection& playerDirection)
{

     foreach(const Platform& platform, platformList)
     {
        int x,y;

        Color platformColor;
        if (playerDirection == platform.getTimeDirection())
        {
            x = platform.getX();
            y = platform.getY();
            platformColor = Color(50,0,0);
        }
        else
        {
            x = platform.getX()-platform.getXspeed();
            y = platform.getY()-platform.getYspeed();
            platformColor = Color(0,0,50);
        }
        //cout << x << " " << y << " " << platform.getXspeed() << " " << platform.getYspeed() << endl;

        target.Draw(Shape::Rectangle(
            x/100,
            y/100,
            (x+platform.getWidth())/100,
            (y+platform.getHeight())/100,
            platformColor)
        );
     }
}

void DrawTimeline(RenderTarget& target, TimeEngine::FrameListList& waves, NewFrameID& playerFrame)
{
    bool pixelsWhichHaveBeenDrawnIn[640] = {false};
    foreach(const FrameUpdateSet& lists, waves) {
        foreach (NewFrameID frame, lists) {
            if (frame.isValidFrame()) {
                if (!pixelsWhichHaveBeenDrawnIn[static_cast<unsigned int> ((frame.frame()/10800.f)*640)]) {
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
    if(playerFrame.isValidFrame()) {
        target.Draw(Shape::Rectangle((playerFrame.frame()/10800.f)*640-1,
                                             10,
                                             (playerFrame.frame()/10800.f)*640+2,
                                             25,
                                             Color(200,200,0)));
    }
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
    //newObjectList.addBox(Box(46400, 15600, -1000, -500, 3200, FORWARDS));
    //newObjectList.addBox(Box(6400, 15600, 1000, -500, 3200, FORWARDS));
    newObjectList.addBox(Box(56400, 15600, 0, 0, 3200, FORWARDS));
    newObjectList.addGuy(Guy(8700, 20000, 0, 0, 1600, 3200, false, false, 0, INVALID, FORWARDS, 0, 0));
    newObjectList.addButton(Button(30400, 44000, 0, 0, 0, false, REVERSE));
    newObjectList.addPlatform(Platform(38400, 44800, 0, 0, 6400, 1600, 0, FORWARDS));

    return TimeEngine
           (
                3,
                10800,
                wall,
                3200,
                50,
                ObjectList(newObjectList),
                NewFrameID(0,10800),
                AttachmentMap
                (
                    ::std::vector< ::boost::tuple<int, int, int> >(1,::boost::tuple<int, int, int>(0,3200,-800))),
                    TriggerSystem
                    (
                        ::std::vector<int>(1, 0),
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
                                20))));
}
}
