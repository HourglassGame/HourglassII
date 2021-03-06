#ifndef HG_LEVEL_LOADER_H
#define HG_LEVEL_LOADER_H
#include "Level.h"
#include "OperationInterrupter.h"
#include "as_lvalue.h"
#include <boost/filesystem/path.hpp>
namespace hg {
    Level loadLevelFromFile(
        boost::filesystem::path const &levelPath, OperationInterrupter &interrupter = as_lvalue(NullOperationInterrupter()));
}
#endif //HG_LEVEL_LOADER_H
