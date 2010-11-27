#ifndef HG_PLAYING_GAME_STATE_H
#define HG_PLAYING_GAME_STATE_H
#include "State.h"
#include "TimeEngine.h"
#include "Hg_Input.h"
#include "LevelDrawer.h"
#include "UIManager.h"
#include <memory>
namespace hg {
    class GameEngine;
    class PlayingGameState : public State {
    public:
        PlayingGameState(::std::auto_ptr<State>& currentState, GameEngine& engine, const Level& level);
        //called whenever the state becomes active after another state was previously active
        //Allows re-polling of settings, window setup, player progress etc without requiring such polling to occur every frame
        void init();
        //called once a frame
        void update();
    private:
        ::std::auto_ptr<State>& currentState_; //Reference to the auto_ptr holding this so that this can be changed or deleted at will
        const Level& level_; //reference kept here so that other states can be constructed with it
        GameEngine& engine_;
        TimeEngine timeEngine_;
        LevelDrawer levelDrawer_;
        //UIManager UIManager_;
        //LevelView view_;
        Input input_; //Should be `InputTranslator' which gets initialised with the user's preferences
        ~PlayingGameState(){}
        //Intentionally undefined
        PlayingGameState(const PlayingGameState& other);
        PlayingGameState& operator=(const PlayingGameState& other);
    };
}
#endif //HG_PLAYING_GAME_STATE_H
