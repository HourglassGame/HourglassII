#ifndef HG_LOADING_GAME_STATE_H
#define HG_LOADING_GAME_STATE_H
#include "State.h"
#include <memory>
namespace hg {
    class GameEngine;
    class Level;
    class LoadingGameState : public State {
        public:
        LoadingGameState(::std::auto_ptr<State>& currentState, GameEngine& engine, const Level& level);
        //called whenever the state becomes active after another state was previously active
        //Allows re-polling of settings, window setup, player progress etc without requiring such polling to occur every frame
        void init();
        //called once a frame
        void update();
        private:
        ::std::auto_ptr<State>& currentState_;
        GameEngine& engine_;
        const Level& level_;
    };
}
#endif //HG_LOADING_GAME_STATE_H

