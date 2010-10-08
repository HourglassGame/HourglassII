#ifndef HG_SPLASH_SCREEN_STATE_H
#define HG_SPLASH_SCREEN_STATE_H
#include "State.h"
namespace hg {
    class GameEngine;
    class SplashScreenState : public State {
    public:
        SplashScreenState(GameEngine& engine);
        void init();
        void update();
    private:
        GameEngine& engine_;
        bool finished;
        //Intentionally undefined
        SplashScreenState(const SplashScreenState& other);
        SplashScreenState& operator=(const SplashScreenState& other);
    };
}
#endif //HG_SPLASH_SCREEN_STATE_H
