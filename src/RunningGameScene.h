#ifndef HG_RUNNING_GAME_SCENE_H
#define HG_RUNNING_GAME_SCENE_H

#include "SceneImplementation.h"
#include "Scene.h"
#include "RenderWindow.h"
#include "variant.h"
#include "move_function.h"
#include "LoadedLevel.h"
namespace hg{
    variant<WindowClosed_tag, GameAborted_tag, GameWon_tag, ReloadLevel_tag, move_function<std::vector<hg::InputList>()>>
    run_game_scene(hg::RenderWindow &window, LoadedLevel &&loadedLevel, std::vector<hg::InputList> const& replay = {});
}
#endif //HG_RUNNING_GAME_SCENE
