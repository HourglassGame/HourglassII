#ifndef HG_RESOURCE_MANAGER_H
#define HG_RESOURCE_MANAGER_H
#include <boost/container/map.hpp>
#include <string>

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include "Environment.h"

namespace hg {
extern sf::Font const *defaultFont;



struct LevelResources {
    boost::container::map<std::string, sf::Image> images;
    boost::container::map<std::string, sf::SoundBuffer> sounds;
};


struct LevelUIData {
    LevelResources resources;
    sf::Image wallImage;
    sf::Image guyPositionImage;
};

//MUST BE CALLED ON MAIN THREAD (since it uses sf::Texture)
//(Currently it's not always called on the main thread... it seems to mostly work)
//Should probably start getting rid of most of SFML, since it can't handle this stuff properly.
LevelResources loadLevelResources(std::string const &levelPath, std::string const &globalsPath);
sf::Image loadAndBakeWallImage(Wall const &wall);
}

#endif // HG_RESOURCE_MANAGER_H
