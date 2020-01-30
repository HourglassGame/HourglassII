#ifndef HG_RUNTIME_ERROR_SCENE_H
#define HG_RUNTIME_ERROR_SCENE_H
#include "RenderWindow.h"
#include "LuaError.h"
#include "GLFWWindow.h"

namespace hg {
    void report_runtime_error(hg::RenderWindow &window, GLFWWindow &windowglfw, VulkanEngine& vulkanEng, VulkanRenderer& vkRenderer, LuaError const &e);
}
#endif //HG_RUNTIME_ERROR_SCENE_H
