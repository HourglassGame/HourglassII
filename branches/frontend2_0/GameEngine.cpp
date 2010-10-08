#include "GameEngine.h"
#include "Loaders.h"
#include "SplashScreenState.h"
#include "MainMenuState.h"
using namespace ::sf;
namespace hg {
GameEngine::GameEngine() :
stateManager(),
options(),
window()
{
}
void GameEngine::go()
{
    stateManager.push(new MainMenuState(*this));
    if (options.displaysplash) {
        stateManager.push(new SplashScreenState(*this));
    }
    loop();
}
void GameEngine::loop()
{
    while (!stateManager.empty()) {
        //Pass in updateManager and make it otherwise inaccesable?
        stateManager.getCurrentState().update(/*updateManager*/);
        //TODO - design flexible and safe update method
        // (can't freeze, allows differnt frame rates,
        // doesn't do unnecessecary calls to display,
        // doesn't duplicate code etc)
    }
}
}
