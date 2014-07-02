#ifndef HG_LOADING_LEVEL_SCENE_H
#define HG_LOADING_LEVEL_SCENE_H
#include "move_function.h"
#include "Level.h"
#include "OperationInterrupter.h"
#include "variant.h"
#include "LoadedLevel.h"
#include "Scene.h"
namespace hg {
    struct RenderWindow;
    variant<hg::LoadedLevel, LoadingCanceled_tag> load_level_scene(
        hg::RenderWindow &window,
        LoadLevelFunction const &levelLoadingFunction);
}

#endif // HG_LOADING_LEVEL_SCENE_H
