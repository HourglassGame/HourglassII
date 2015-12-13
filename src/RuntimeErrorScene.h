#ifndef HG_RUNTIME_ERROR_SCENE_H
#define HG_RUNTIME_ERROR_SCENE_H
#include "RenderWindow.h"
#include "LuaError.h"
namespace hg {
    void report_runtime_error(hg::RenderWindow &window, LuaError const &e);
}
#endif //HG_RUNTIME_ERROR_SCENE_H
