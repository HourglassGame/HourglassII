#ifndef HG_LEVEL_LOADER_H
#define HG_LEVEL_LOADER_H
#include "Level.h"
#include "OperationInterrupter.h"
#include <boost/filesystem/path.hpp>
namespace hg {
    Level loadLevelFromFile(
        boost::filesystem::path const &levelPath, OperationInterrupter &interrupter);
}
#endif //HG_LEVEL_LOADER_H
