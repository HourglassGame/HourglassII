#include "SplashScreenState.h"
#include "GameEngine.h"
#include "Loaders.h"
#include "Utility.h"
#include "Position.h"
#include <SFML/System.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/String.hpp>
using ::sf::String;
using ::sf::Sprite;
using ::sf::Event;
using ::sf::Sleep;
using ::sf::Image;
using ::sf::VideoMode;
using namespace ::sf::Style;
using ::sf::Font;
namespace hg {
SplashScreenState::SplashScreenState(GameEngine& engine) :
engine_(engine),
finished(false)
{
}
void SplashScreenState::init()
{
    Image splashImage(loadImageAcceptFail("splashscreen.png", 640, 480));
    Sprite splash(splashImage);

    engine_.window.Create(VideoMode((int)splash.GetSize().x, (int)splash.GetSize().y),"",None);
    engine_.window.Clear();
    engine_.window.Draw(splash);
    {
            String pressAnyKey("Press any Key...");
            Font ruritania(loadFontAcceptFail("Ruritania.ttf"));
            pressAnyKey.SetFont(ruritania);
            pressAnyKey.SetSize(20.f);
            setCentre(pressAnyKey, horizontalposition::MIDDLE(),verticalposition::MIDDLE());
            pressAnyKey.SetPosition(engine_.window.GetWidth()/2, engine_.window.GetHeight()-40);
            engine_.window.Draw(pressAnyKey);
    }
    engine_.window.Display();
}
void SplashScreenState::update()
{
    Event event;
    while (engine_.window.GetEvent(event)){
        if (event.Type == Event::KeyPressed || event.Type == Event::MouseButtonPressed) {
            engine_.stateManager.pop();
            return;
        }
    }
    Sleep(0.25f);
}
}
