#ifndef HG_LEVEL_LOADER_H
#define HG_LEVEL_LOADER_H
#include "Level.h"
#include "OperationInterrupter.h"
#include <string>
namespace hg {
Level loadLevelFromFile(
    std::string const& filename);
} //namespace hg
#endif //HG_LEVEL_LOADER_H
