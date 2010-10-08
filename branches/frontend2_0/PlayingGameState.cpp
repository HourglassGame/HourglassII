#include "PlayingGameState.h"

#include "GameEngine.h"
namespace hg {
PlayingGameState::PlayingGameState(::std::auto_ptr<State>& currentState, GameEngine& engine, const Level& level) :
currentState_(currentState),
level_(level),
engine_(engine),
timeEngine_(level),
levelDrawer(level)
{
}
}
