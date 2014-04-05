#include "LoadedLevel.h"
#include "LevelLoader.h"
namespace hg {
    LoadedLevel loadLevelAndResourcesFromFile(boost::filesystem::path const &path, OperationInterrupter &interrupter) {
        auto timeEngine = TimeEngine(loadLevelFromFile(path.string(),interrupter), interrupter);
        auto wallImage = loadAndBakeWallImage(timeEngine.getWall());
        return {
            std::move(timeEngine),
            loadLevelResources(path.string(), "GlitzData"),
            std::move(wallImage)};
    }
}
