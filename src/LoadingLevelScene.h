#ifndef HG_LOADING_LEVEL_SCENE_H
#define HG_LOADING_LEVEL_SCENE_H
#include "move_function.h"
#include "Level.h"
#include "OperationInterrupter.h"
#include <variant>
#include "LoadedLevel.h"
#include "Scene.h"
#include "GLFWWindow.h"

namespace hg {
    struct RenderWindow;
    std::variant<hg::LoadedLevel, LoadingCanceled_tag> load_level_scene(
        hg::RenderWindow &window,
        GLFWWindow &windowglfw,
        LoadLevelFunction const &levelLoadingFunction,
        VulkanEngine& vulkanEng,
        VulkanRenderer& vkRenderer);
}

#endif // HG_LOADING_LEVEL_SCENE_H
