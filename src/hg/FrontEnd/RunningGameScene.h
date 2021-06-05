#ifndef HG_RUNNING_GAME_SCENE_H
#define HG_RUNNING_GAME_SCENE_H

#include "Scene.h"
#include <variant>
#include "hg/Util/move_function.h"
#include "LoadedLevel.h"
#include "VulkanEngine.h"
#include "VulkanRenderer.h"
#include "GLFWWindow.h"

namespace hg{
	std::variant<GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>>
	run_game_scene(GLFWWindow &windowglfw, VulkanEngine &eng, VulkanRenderer &vkRenderer, LoadedLevel &&loadedLevel, std::vector<hg::InputList> const& replay = {});
}
#endif //HG_RUNNING_GAME_SCENE
