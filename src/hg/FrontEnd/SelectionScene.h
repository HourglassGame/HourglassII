#ifndef HG_SELECTION_SCENE_H
#define HG_SELECTION_SCENE_H

#include "Scene.h"
#include "GLFWWindow.h"
#include <variant>

#include <vector>
#include <string>
namespace hg
{
	struct LevelState {
		std::string name;
		int completeThreshold;
	};

	struct PageState {
		std::string name;
		int nextUnlockThreshold;
		std::vector<LevelState> options;
	};
	
	std::variant<std::size_t, SceneAborted_tag> run_selection_scene(
		GLFWWindow &windowglfw,
		int defaultOption,
		std::vector<std::string> const &options,
		VulkanEngine& vulkanEng,
		VulkanRenderer& vkRenderer);
	std::variant<std::size_t, SceneAborted_tag> run_selection_page_scene(
		GLFWWindow &windowglfw,
		int defaultOption,
		int perPage,
		std::vector<PageState> const &levelMenuConf,
		VulkanEngine& vulkanEng,
		VulkanRenderer& vkRenderer);
}
#endif

