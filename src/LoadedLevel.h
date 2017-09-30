#ifndef HG_LOADED_LEVEL_H
#define HG_LOADED_LEVEL_H
#include "TimeEngine.h"
#include "ResourceManager.h"
#include <boost/filesystem/path.hpp>
#include <SFML/Graphics/Image.hpp>
#include "OperationInterrupter.h"
#include "as_lvalue.h"
namespace hg{
    struct LoadedLevel final {
        TimeEngine timeEngine;
        LevelResources resources;
        std::unique_ptr<sf::Image> bakedWall;
    };
    LoadedLevel loadLevelAndResourcesFromFile(
        boost::filesystem::path const &path,
        OperationInterrupter &interrupter = as_lvalue(NullOperationInterrupter{}));
}
#endif //HG_LOADED_LEVEL_H
