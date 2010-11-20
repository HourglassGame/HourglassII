#include "Loaders.h"
#include "Level.h"
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Font.hpp>
#include <boost/assign.hpp>
#include <boost/foreach.hpp>
#include <iostream>
#include <cassert>
#define foreach BOOST_FOREACH
using ::sf::Image;
using ::sf::Color;
using ::sf::Font;
using ::std::string;
namespace hg {
    Image loadImageAcceptFail(string filename, int widthOnFail, int heightOnFail)
    {
        Image retv;
        if (!retv.LoadFromFile(filename)) {
            retv.Create(widthOnFail, heightOnFail, Color(255,0,255));
        }
        return retv;
    }
    Font loadFontAcceptFail(string filename)
    {
        Font retv;
        retv.LoadFromFile(filename);
        return retv;
    }
    Level loadLevelAcceptFail(const ::std::string& filename)
    {
        assert(filename.size() == 0 && "anything else is setting sail for fail");
        using ::std::vector;
        using ::boost::tuple;
        
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
        
        MutableObjectList newObjectList;
        newObjectList.addBox(Box(32000, 15600, 0, 0, 3200, FORWARDS, 0));
        //newObjectList.addBox(Box(46400, 15600, -1000, -500, 3200, FORWARDS, 0));
        //newObjectList.addBox(Box(6400, 15600, 1000, -500, 3200, FORWARDS, 0));
        //newObjectList.addBox(Box(56400, 15600, 0, 0, 3200, FORWARDS, 0));
        newObjectList.addGuy(Guy(8700, 20000, 0, 0, 1600, 3200, false, false, 0, INVALID, 0, FORWARDS, 0, 0, 0));
        newObjectList.addButton(Button(30400, 44000, 0, 0, 0, false, REVERSE, 0));
        newObjectList.addPlatform(Platform(38400, 44800, 0, 0, 6400, 1600, 0, FORWARDS, 0));

        Level retv (3,
        10800,
        actualWall,
        3200,
        30,
        ObjectList(newObjectList),
        NewFrameID(0,10800),
        AttachmentMap
        (
            ::std::vector< ::boost::tuple<int, int, int> >(1,::boost::tuple<int, int, int>(0,3200,-800))
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
        return retv;
    }
}

