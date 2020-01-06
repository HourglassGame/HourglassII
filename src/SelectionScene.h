#ifndef HG_SELECTION_SCENE_H
#define HG_SELECTION_SCENE_H

#include "Scene.h"
#include "RenderWindow.h"
#include "GLFWWindow.h"
#include <variant>

#include <vector>
#include <string>
namespace hg
{
    std::variant<std::size_t, SceneAborted_tag> run_selection_scene(
        hg::RenderWindow &window,
        GLFWWindow &windowglfw,
        std::vector<std::string> const &options,
        VulkanEngine& vulkanEng,
        VulkanRenderer& vkRenderer);
}
#endif

