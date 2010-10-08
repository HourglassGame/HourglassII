#ifndef HG_MAIN_MENU_STATE_H
#define HG_MAIN_MENU_STATE_H
#include "State.h"
#include "Menu.h"
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/String.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
namespace hg {
    class GameEngine;
    class MainMenuState : public State {
    public:
        MainMenuState(GameEngine& engine);
        void init();
        void update();
    private:
        GameEngine& engine_;
        ::sf::Image background;
        ::sf::Sprite backgroundSprite;
        bool needsUpdate;
        ::sf::Font ruritania;
        /*::sf::String version;*/
        /*::sf::String title;*/
        Menu<horizontalposition::RIGHT, verticalposition::TOP> menu;
        

        //actions for buttons
        void exit();
        void play();
        
        //utility functions
        void scaleBackgroundSprite();
        
        template <typename BoundActionType>
        ActiveMenuItem< ::sf::RenderTarget, ::sf::String, BoundActionType>* makeItem(::std::string labelString,
                                                                                     const BoundActionType& action) const;
        //Intentionally undefined
        MainMenuState(const MainMenuState& other);
        MainMenuState& operator=(const MainMenuState& other);
    };
}
#endif //HG_MAIN_MENU_STATE_H
