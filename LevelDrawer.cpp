#include "LevelDrawer.h"
#include "ObjectList.h"
#include "Level.h"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>
#include <boost/foreach.hpp>
#include <boost/multi_array.hpp>
#include <vector>
#define foreach BOOST_FOREACH
namespace hg {
LevelDrawer::LevelDrawer(const Level& nlevel) :
level(nlevel)
{
}
static void drawWall(sf::RenderTarget& target, const boost::multi_array<bool, 2>& wall)
{
    target.Clear(sf::Color(255,255,255));
    for(unsigned int i = 0; i < wall.shape()[0]; ++i) {
        for(unsigned int j = 0; j < wall.shape()[1]; ++j) {
            if (wall[i][j]) {
                target.Draw
                (
                    sf::Shape::Rectangle
                    (
                        32u*i,
                        32u*j,
                        32u*(i+1),
                        32u*(j+1),
                        sf::Color()
                    )
                );
            }
        }
    }
}

static void drawBoxes(sf::RenderTarget& target, const std::vector<Box>& boxList, TimeDirection& playerDirection)
{
    foreach(const Box& box, boxList) {
        if (playerDirection == box.getTimeDirection())
        {
             target.Draw(sf::Shape::Rectangle(
                box.getX()/100,
                box.getY()/100,
                (box.getX()+ box.getSize())/100,
                (box.getY()+box.getSize())/100,
                sf::Color(255,0,255))
            );
        }
        else
        {
            int x = box.getX()-box.getXspeed();
            int y = box.getY()-box.getYspeed();
            target.Draw(sf::Shape::Rectangle(
                x/100,
                y/100,
                (x+ box.getSize())/100,
                (y+box.getSize())/100,
                sf::Color(0,255,0))
            );
        }
    }
}

static void drawGuys(sf::RenderTarget& target, const std::vector<Guy>& guyList, TimeDirection& playerDirection)
{
    foreach(const Guy& guy, guyList) {
        int x,y;
        sf::Color guyColor;
        if (playerDirection == guy.getTimeDirection())
        {
            x = guy.getX();
            y = guy.getY();
            guyColor = sf::Color(150,150,0);
        }
        else
        {
            x = guy.getX()-guy.getXspeed();
            y = guy.getY()-guy.getYspeed();
            guyColor = sf::Color(0,0,150);
        }

        target.Draw(sf::Shape::Rectangle(
            x/100,
            y/100,
            (x+ guy.getWidth())/100,
            (y+guy.getHeight())/100,
            guyColor)
        );

        if (guy.getBoxCarrying())
        {
            sf::Color boxColor;
            if (playerDirection == guy.getBoxCarryDirection())
            {
                boxColor = sf::Color(150,0,150);
            }
            else
            {
                boxColor = sf::Color(0,150,0);
            }

            target.Draw(sf::Shape::Rectangle(
                (x + guy.getWidth()/2 - guy.getBoxCarrySize()/2)/100,
                (y - guy.getBoxCarrySize())/100,
                (x + guy.getWidth()/2 + guy.getBoxCarrySize()/2)/100,
                y/100,
                boxColor)
            );
        }
    }
}

static void drawButtons(sf::RenderTarget& target, const std::vector<Button>& buttonList, TimeDirection& playerDirection)
{
     foreach(const Button& button, buttonList)
     {
        sf::Color buttonColor;
        if (button.getState())
        {
            buttonColor = sf::Color(150,255,150);
        }
        else
        {
            buttonColor = sf::Color(255,150,150);
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

        target.Draw(sf::Shape::Rectangle(
            x/100,
            y/100,
            (x+3200)/100,
            (y+800)/100,
            buttonColor)
        );
     }
}

static void drawPlatforms(sf::RenderTarget& target, const std::vector<Platform>& platformList, TimeDirection& playerDirection)
{

     foreach(const Platform& platform, platformList)
     {
        int x,y;

        sf::Color platformColor;
        if (playerDirection == platform.getTimeDirection())
        {
            x = platform.getX();
            y = platform.getY();
            platformColor = sf::Color(50,0,0);
        }
        else
        {
            x = platform.getX()-platform.getXspeed();
            y = platform.getY()-platform.getYspeed();
            platformColor = sf::Color(0,0,50);
        }
        //cout << x << " " << y << " " << platform.getXspeed() << " " << platform.getYspeed() << endl;

        target.Draw(sf::Shape::Rectangle(
            x/100,
            y/100,
            (x+platform.getWidth())/100,
            (y+platform.getHeight())/100,
            platformColor)
        );
     }
}

void LevelDrawer::draw(::sf::RenderTarget& target, const ObjectList& objectList, TimeDirection viewDirection)
{
    drawWall(target, level.wallmap);
    drawBoxes(target, objectList.getBoxListRef(), viewDirection);
    drawGuys(target, objectList.getGuyListRef(), viewDirection);
    drawButtons(target, objectList.getButtonListRef(), viewDirection);
    drawPlatforms(target, objectList.getPlatformListRef(), viewDirection);
}
}
