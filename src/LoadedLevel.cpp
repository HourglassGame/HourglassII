#include "LoadedLevel.h"
#include "LevelLoader.h"
namespace hg {
#if 0
    LoadedLevel loadLevelAndResourcesFromFile(boost::filesystem::path const &path, OperationInterrupter &interrupter) {
        auto timeEngine = TimeEngine(loadLevelFromFile(path.string(),interrupter), interrupter);
        auto wallImage = std::make_unique<sf::Image>(loadAndBakeWallImage(timeEngine.getWall()));
        auto positionColourImage = std::make_unique<sf::Image>(loadAndBakePositionColourImage(timeEngine.getWall()));
        return {
            std::move(timeEngine),
            loadLevelResources(path.string(), "GlitzData"),
            std::move(wallImage),
            std::move(positionColourImage)};
    }
#endif
}
