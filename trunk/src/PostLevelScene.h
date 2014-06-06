#ifndef HG_POST_LEVEL_SCENE_H
#define HG_POST_LEVEL_SCENE_H
#include "Scene.h"
namespace hg {
    struct RenderWindow;
    struct LoadedLevel;
    void run_post_level_scene(
        hg::RenderWindow &window,
        LoadedLevel const &initialLevel,
        LoadedLevel const &finalLevel);
}
#endif // HG_POST_LEVEL_SCENE_H
