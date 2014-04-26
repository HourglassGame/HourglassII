#ifndef HG_LOADING_LEVEL_SCENE_H
#define HG_LOADING_LEVEL_SCENE_H
#include "SceneImplementation.h"
#include "move_function.h"
#include "Level.h"
#include "OperationInterrupter.h"
#include "variant.h"
#include "LoadedLevel.h"
#include "Scene.h"
namespace hg {
    variant<hg::LoadedLevel, LoadingCanceled_tag> load_level_scene(
        hg::RenderWindow &window,
        hg::move_function<hg::LoadedLevel(hg::OperationInterrupter &)> const& levelLoadingFunction);
}

#endif // HG_LOADING_LEVEL_SCENE_H
