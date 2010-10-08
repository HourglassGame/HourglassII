#include "GameState.h"
#include "Utility.h"
namespace hg {
GameState::GameState(GameEngine& engine, const Level& nlevel) :
engine_(engine),
level(nlevel),
currentState(0/*new PlayingGameState(currentState, engine, level)*/)
{
}

void GameState::init()
{
    initialiseNormalWindow(engine_.window, engine_.options);
    if (currentState.get()) {
        currentState->init();
    }
}
void GameState::update()
{
    if (currentState.get()) {
        currentState->update();
    }
    else {
        engine_.stateManager.pop();
    }
}
}
