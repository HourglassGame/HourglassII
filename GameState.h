#ifndef HG_GAME_STATE_H
#define HG_GAME_STATE_H
#include "State.h"
#include "GameEngine.h"
#include "TimeEngine.h"
#include "Level.h"
#include <memory>
namespace hg {
class GameState : public State {
public:
GameState(GameEngine& engine, const Level& nlevel);
//called whenever the state becomes active after another state was previously active
//Allows re-polling of settings, window setup, player progress etc without requiring such polling to occur every frame
void init();
//called once a frame
void update();
~GameState(){}
private:
GameEngine& engine_;
public:
Level level;
::std::auto_ptr<State> currentState;
private:

//Intentionally undefined
GameState(const GameState& other);
GameState& operator=(const GameState& other);
};
}
   
#endif //HG_GAME_STATE_H
