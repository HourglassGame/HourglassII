#ifndef HG_LEVEL_SELECTION_SCENE_H
#define HG_LEVEL_SELECTION_SCENE_H
#include "move_function.h"
#include "Scene.h"
#include "variant.h"
#include "LoadedLevel.h"
#include "OperationInterrupter.h"
#include "RenderWindow.h"
namespace hg {
    variant<hg::move_function<hg::LoadedLevel(hg::OperationInterrupter &)>, WindowClosed_tag, SceneAborted_tag>
    run_level_selection_scene(hg::RenderWindow &window);
}
#endif //HG_LEVEL_SELECTION_SCENE_H
