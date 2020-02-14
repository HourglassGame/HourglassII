#ifndef HG_LEVEL_SELECTION_SCENE_H
#define HG_LEVEL_SELECTION_SCENE_H
#include "hg/Util/move_function.h"
#include "Scene.h"
#include <variant>
#include "LoadedLevel.h"
#include "hg/Util/OperationInterrupter.h"
#include "GLFWWindow.h"

namespace hg {
    std::variant<LoadLevelFunction, SceneAborted_tag>
    run_level_selection_scene(
        GLFWWindow &windowglfw,
        VulkanEngine& vulkanEng,
        VulkanRenderer& vkRenderer,
        std::string &levelName);
}
#endif //HG_LEVEL_SELECTION_SCENE_H
