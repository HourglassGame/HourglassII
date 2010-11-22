#include "GameState.h"
#include "Utility.h"
#include "PlayingGameState.h"
#include "SplashScreenState.h"
#include <iostream>
namespace hg {
GameState::GameState(GameEngine& engine, const Level& level) :
engine_(engine),
level_(level),
currentState_(new PlayingGameState(currentState_, engine, level_))
{
}

void GameState::init()
{
    initialiseNormalWindow(engine_.window, engine_.options);
    if (currentState_.get()) {
        currentState_->init();
    }
}
void GameState::update()
{
    if (currentState_.get()) {
        currentState_->update();
    }
    else {
        engine_.stateManager.pop();
    }
}
}
