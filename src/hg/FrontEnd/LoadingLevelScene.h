#ifndef HG_LOADING_LEVEL_SCENE_H
#define HG_LOADING_LEVEL_SCENE_H
#include "hg/Util/move_function.h"
#include "hg/TimeEngine/Level.h"
#include "hg/Util/OperationInterrupter.h"
#include <variant>
#include "LoadedLevel.h"
#include "Scene.h"
#include "GLFWWindow.h"

namespace hg {
	std::variant<hg::LoadedLevel, LoadingCanceled_tag> load_level_scene(
		GLFWWindow &windowglfw,
		LoadLevelFunction const &levelLoadingFunction,
		VulkanEngine& vulkanEng,
		VulkanRenderer& vkRenderer);
}

#endif // HG_LOADING_LEVEL_SCENE_H
