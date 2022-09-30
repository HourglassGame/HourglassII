#ifndef HG_SELECTION_SCENE_H
#define HG_SELECTION_SCENE_H

#include "Scene.h"
#include "GLFWWindow.h"
#include <variant>

#include <vector>
#include <string>
namespace hg
{
	struct LevelState final {
		std::string name;
		std::string humanName;
		std::variant<int, std::string> unlockRequirement;
	};

	struct PageState final {
		std::string name;
		int prevLevelsRequired;
		std::vector<LevelState> options;
	};
	
	struct SelectionPageFrameState {
		int selectedItem;
		int page;
		std::vector<PageState> pages;
		bool unlockAll;
	};

	struct LevelSelectionReturn {
		std::string name;
		int position;
		int page;
	};

	std::variant<std::size_t, SceneAborted_tag> run_selection_scene(
		GLFWWindow &windowglfw,
		int defaultOption,
		std::vector<std::string> const &options,
		VulkanEngine& vulkanEng,
		VulkanRenderer& vkRenderer);
	std::variant<LevelSelectionReturn, SceneAborted_tag> run_selection_page_scene(
		GLFWWindow &windowglfw,
		int defaultOption,
		int defaultPage,
		std::vector<PageState> const &levelMenuConf,
		VulkanEngine& vulkanEng,
		VulkanRenderer& vkRenderer,
		bool unlockAll);
}
#endif

