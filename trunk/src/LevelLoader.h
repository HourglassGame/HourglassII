#ifndef HG_LEVEL_LOADER_H
#define HG_LEVEL_LOADER_H
#include "Level.h"
#include "OperationInterruptor.h"
#include <string>
namespace hg {
Level loadLevelFromFile(
    std::string const& filename,
    OperationInterruptor const& interruptor = g_nullInterruptor);
} //namespace hg
#endif //HG_LEVEL_LOADER_H