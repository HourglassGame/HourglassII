#ifndef HG_RESOURCE_MANAGER_H
#define HG_RESOURCE_MANAGER_H
#include <boost/container/map.hpp>
#include <string>

#include <SFML/Graphics/Image.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include "Environment.h"

namespace hg {

struct LevelResources final {
    boost::container::map<std::string, sf::Image> images;
    boost::container::map<std::string, sf::SoundBuffer> sounds;
};


struct LevelUIData final {
    LevelResources resources;
};

LevelResources loadLevelResources(std::string const &levelPath, std::string const &globalsPath);
}

#endif // HG_RESOURCE_MANAGER_H
