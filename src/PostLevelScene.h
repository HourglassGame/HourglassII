#ifndef HG_POST_LEVEL_SCENE_H
#define HG_POST_LEVEL_SCENE_H
#include "Scene.h"
#include "VulkanEngine.h"
#include "GLFWWindow.h"

namespace hg {
    struct RenderWindow;
    struct LoadedLevel;
    class TimeEngine;
    void run_post_level_scene(
        hg::RenderWindow &window,
        GLFWWindow &windowglfw,
        hg::VulkanEngine &eng,
        TimeEngine const &initialTimeEngine,
        LoadedLevel const &finalLevel);
}
#endif // HG_POST_LEVEL_SCENE_H
