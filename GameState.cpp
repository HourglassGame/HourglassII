#include "GameState.h"
#include "Utility.h"
#include "PlayingGameState.h"
#include "SplashScreenState.h"
#include <iostream>
namespace hg {
GameState::GameState(GameEngine& engine, const Level& nlevel) :
engine_(engine),
level(nlevel),
currentState(new PlayingGameState(currentState, engine, level))
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
