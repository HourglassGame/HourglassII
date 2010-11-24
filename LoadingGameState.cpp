#include "LoadingGameState.h"
#include "PlayingGameState.h"
#include "GameEngine.h"
#include "Utility.h"
#include "Loaders.h"
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/String.hpp>
#include <algorithm>
namespace hg {
    LoadingGameState::LoadingGameState(::std::auto_ptr<State>& currentState, GameEngine& engine, const Level& level) :
    currentState_(currentState),
    engine_(engine),
    level_(level)
    {
    }
    //called whenever the state becomes active after another state was previously active
    //Allows re-polling of settings, window setup, player progress etc without requiring such polling to occur every frame
    void LoadingGameState::init(){}
    //called once a frame
    void LoadingGameState::update()
    {
        {
            sf::String loading("Initialising Reality...");
            sf::Font ruritania(loadFontAcceptFail("Ruritania.ttf"));
            loading.SetFont(ruritania);
            loading.SetSize(20.f);
            scale(loading,::std::min(engine_.window.GetWidth()*0.25f/getXSize(loading), engine_.window.GetHeight()*0.25f/getYSize(loading)));
            loading.SetPosition(engine_.window.GetWidth() - (getXSize(loading) + engine_.window.GetWidth()/10.f),
                                engine_.window.GetHeight() - (getYSize(loading) + engine_.window.GetHeight()/10.f));
            engine_.window.Clear(::sf::Color(0,0,0));
            engine_.window.Draw(loading);
            engine_.window.Display();
        }
        ::std::auto_ptr<State> newState(new PlayingGameState(currentState_,engine_,level_));
        newState->init();
        currentState_ = newState;
    }       
}
