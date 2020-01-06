#ifndef HG_LEVEL_SELECTION_SCENE_H
#define HG_LEVEL_SELECTION_SCENE_H
#include "move_function.h"
#include "Scene.h"
#include <variant>
#include "LoadedLevel.h"
#include "OperationInterrupter.h"
#include "RenderWindow.h"
#include "GLFWWindow.h"

namespace hg {
    std::variant<LoadLevelFunction, SceneAborted_tag>
    run_level_selection_scene(
        hg::RenderWindow &window,
        GLFWWindow &windowglfw,
        VulkanEngine& vulkanEng,
        VulkanRenderer& vkRenderer);
}
#endif //HG_LEVEL_SELECTION_SCENE_H
