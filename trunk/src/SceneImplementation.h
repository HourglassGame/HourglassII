#ifndef HG_SCENE_IMPLEMENTATION_H
#define HG_SCENE_IMPLEMENTATION_H
#include <vector>
#include <memory>
namespace hg {
    struct Scene;
    struct RenderWindow;
    struct SceneImplementation {
        virtual std::vector<Scene> run(std::unique_ptr<SceneImplementation> &me, hg::RenderWindow &window) = 0;
        virtual ~SceneImplementation(){}
    };
}

#endif // HG_SCENE_IMPLEMENTATION_H
