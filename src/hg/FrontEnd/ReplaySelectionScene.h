#ifndef HG_REPLAY_SELECTION_SCENE_H
#define HG_REPLAY_SELECTION_SCENE_H

#include "hg/TimeEngine/ArrivalDepartures/InputList.h"
#include "Scene.h"
#include "GLFWWindow.h"
#include <variant>

#include <vector>
#include <string>
namespace hg {
    std::variant<move_function<std::vector<InputList>()>, SceneAborted_tag> run_replay_selection_scene(
        GLFWWindow &windowglfw,
        std::string const &levelName,
        VulkanEngine& vulkanEng,
        VulkanRenderer& vkRenderer);
}
#endif // HG_REPLAY_SELECTION_SCENE_H