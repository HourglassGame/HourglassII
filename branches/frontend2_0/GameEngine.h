#ifndef HG_GAME_ENGINE_H
#define HG_GAME_ENGINE_H
#include "Options.h"
#include "StateManager.h"
#include "Window.h"
namespace hg {
class GameEngine {
public:
    GameEngine();
    void go();
    void loop();
    StateManager stateManager;
    Options options;
    //Render window here. A multi window or other complicated setup would require a different arrangement 
    //(perhaps a WindowManager class), but that can be dealt with if/when the need occurs
    Window window;
private:
    GameEngine(const GameEngine& other); //Intentionally undefined
    GameEngine& operator=(const GameEngine& other); //Intentionally undefined

};
}
#endif //HG_GAME_ENGINE_H
