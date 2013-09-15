#ifndef HG_SCENE_H
#define HG_SCENE_H
#include "unique_ptr.h"
namespace hg {
    enum Continuation {
        RENDER_SCENE,
        QUIT_GAME,
    };
    struct NextThing {
        NextThing(Continuation continuation);
        NextThing(hg::unique_ptr<Scene> nextScene);
        Continuation continuation;
        hg::unique_ptr<Scene> nextScene;
    };
    struct Scene {
        virtual NextThing processInput(Input input) {
            while (event = input.next()) {
                //do shit
            }
            return NextThing{RenderScene};
        }
        virtual void renderScene(renderTarget target) {
        
        }
        virtual ~Scene() {}
    };
}

#endif //HG_SCENE_H
