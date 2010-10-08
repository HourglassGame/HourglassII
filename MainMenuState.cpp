#include "MainMenuState.h"
#include "Loaders.h"
#include "GameEngine.h"
#include "Utility.h"
#include <SFML/System.hpp>
#include <boost/bind.hpp>
#include <algorithm>
#include <iostream>
using namespace ::sf;
namespace {
    void noop() {}
}
namespace hg {
MainMenuState::MainMenuState(GameEngine& engine) :
engine_(engine),
background(loadImageAcceptFail("HG2TempMenu.png", engine_.options.width, engine_.options.height)),
backgroundSprite(background),
needsUpdate(true),
ruritania(loadFontAcceptFail("Ruritania.ttf")),
menu()
{
    //addItem deletes on fail
    menu.addItem(makeItem("Play", ::boost::bind(&MainMenuState::play, ::boost::ref(*this))));
    menu.addItem(makeItem("View Replays", &noop));
    menu.addItem(makeItem("Run Benchmark", &noop));
    menu.addItem(makeItem("Preferences", &noop));
    menu.addItem(makeItem("Exit", ::boost::bind(&MainMenuState::exit, ::boost::ref(*this))));
}
void MainMenuState::exit() {
    engine_.stateManager.popAll();
}
void MainMenuState::play()
{
}
void MainMenuState::init()
{
    //Ensure that the window is given the right settings given the options
    initialiseNormalWindow(engine_.window, engine_.options);
    scaleBackgroundSprite();
    menu.updatePositions(engine_.window.GetHeight(), engine_.window.GetWidth());
    needsUpdate = true;
}
void MainMenuState::update()
{
    Event event;
    while (engine_.window.GetEvent(event)) {
        switch (event.Type) {
            case Event::Closed:
                engine_.stateManager.popAll();
                return;
            case Event::Resized:
                engine_.window.GetDefaultView().SetFromRect(::sf::FloatRect(0,0,event.Size.Width, event.Size.Height));
                scaleBackgroundSprite();
                menu.updatePositions(engine_.window.GetHeight(), engine_.window.GetWidth());
                engine_.options.width = engine_.window.GetHeight();
                engine_.options.height = engine_.window.GetWidth();
                needsUpdate = true;
                break;
            case Event::MouseButtonPressed:
                if (menu.updatePress(event.MouseButton.X, event.MouseButton.Y)) {
                    return;//Get outta here, we may now be dead
                }
                break;
            default:
                break;
        }
    }
    if (needsUpdate) {
        engine_.window.Clear();
        engine_.window.Draw(backgroundSprite);
        menu.draw(engine_.window);
        engine_.window.Display();
        needsUpdate = false;
    }
    ::sf::Sleep(0.25f);
}
void MainMenuState::scaleBackgroundSprite()
{
    float scaleFactor(::std::max(engine_.window.GetWidth()/(1.f*background.GetWidth()),
                                 engine_.window.GetHeight()/(1.f*background.GetHeight())));
    backgroundSprite.SetScaleX(scaleFactor);
    backgroundSprite.SetScaleY(scaleFactor);
    setCentre(backgroundSprite, horizontalposition::MIDDLE(), verticalposition::MIDDLE());
    backgroundSprite.SetPosition(engine_.window.GetDefaultView().GetCenter());
}
template <typename BoundActionType>
ActiveMenuItem< ::sf::RenderTarget, ::sf::String, BoundActionType>* MainMenuState::makeItem(::std::string labelString, const BoundActionType& action) const
{
    sf::String label(labelString);
    label.SetFont(ruritania);
    return new ActiveMenuItem< ::sf::RenderTarget, ::sf::String, BoundActionType>(label, action);
}
}
