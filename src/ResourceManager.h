#ifndef HG_RESOURCE_MANAGER_H
#define HG_RESOURCE_MANAGER_H
#include <map>
#include <string>

#include <SFML/Graphics/Image.hpp>

namespace hg {
struct LevelResources {
    std::map<std::string, sf::Image> images;
};
//MUST BE CALLED ON MAIN THREAD (since it uses sf::Image)
//Should probably start getting rid of most of SFML, since it can't handle this stuff properly.
LevelResources loadLevelResources(std::string const& levelPath, std::string const& globalsPath);
}

#endif // HG_RESOURCE_MANAGER_H
