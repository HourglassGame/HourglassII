#ifndef HG_MAIN_MENU_SCENE_H
#define HG_MAIN_MENU_SCENE_H
#include "Scene.h"
#include <variant>
#include "VulkanEngine.h"
#include "GLFWWindow.h"

namespace hg {
	std::variant<RunALevel_tag, RunAReplay_tag, Exit_tag> run_main_menu(
		GLFWWindow &windowglfw,
		VulkanEngine &vulkanEng,
		VulkanRenderer &vkRenderer);
}

#endif //HG_MAIN_MENU_SCENE_H
