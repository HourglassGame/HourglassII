#ifndef HG_POST_LEVEL_SCENE_H
#define HG_POST_LEVEL_SCENE_H
#include "Scene.h"
#include "VulkanEngine.h"
#include "GLFWWindow.h"

namespace hg {
    struct LoadedLevel;
    class TimeEngine;
    void run_post_level_scene(
        GLFWWindow &windowglfw,
        hg::VulkanEngine &eng,
        VulkanRenderer& vkRenderer,
        TimeEngine const &initialTimeEngine,
        LoadedLevel const &finalLevel);
}
#endif // HG_POST_LEVEL_SCENE_H
