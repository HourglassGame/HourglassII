#include "Hg_Input.h"
#include "InputList.h"
#include "Maths.h"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
namespace hg
{
static void updatePress(int &var, bool inputState)
{
    var = inputState ?
            var == 0 ? 1 : -1
          : 0;
}

Input::Input() :
    left(),
    right(),
    up(),
    down(),
    space(),
    mouseLeft(),
    abilityCursor(),
    mouseTimelinePosition(),
    mouseX(),
    mouseY(),
    timelineLength()
{
}

void Input::updateState(hg::RenderWindow::InputState const &input, int mouseXOfEndOfTimeline, double mouseScale)
{
    left = input.isKeyPressed(sf::Keyboard::A);
    right = input.isKeyPressed(sf::Keyboard::D);
    up = input.isKeyPressed(sf::Keyboard::W);
    updatePress(down, input.isKeyPressed(sf::Keyboard::S));
    updatePress(space, input.isKeyPressed(sf::Keyboard::Space));
    
    updatePress(mouseLeft, input.isMouseButtonPressed(sf::Mouse::Left));

    if (input.isKeyPressed(sf::Keyboard::Num1)) {
        abilityCursor = Ability::TIME_JUMP;
    }
    if (input.isKeyPressed(sf::Keyboard::Num2)) {
        abilityCursor = Ability::TIME_REVERSE;
    }
    if (input.isKeyPressed(sf::Keyboard::Num3)) {
        abilityCursor = Ability::TIME_GUN;
    }
    if (input.isKeyPressed(sf::Keyboard::Num4)) {
        abilityCursor = Ability::TIME_PAUSE;
    }
    if (input.isMouseButtonPressed(sf::Mouse::Right)) {
        mouseTimelinePosition = flooredModulo(static_cast<int>(input.getMousePosition().x*timelineLength/static_cast<double>(mouseXOfEndOfTimeline)),timelineLength);
    }
    if (input.isMouseButtonPressed(sf::Mouse::Middle)) {
        mouseTimelinePosition = flooredModulo(static_cast<int>(input.getMousePosition().x*timelineLength/static_cast<double>(mouseXOfEndOfTimeline)),timelineLength);
    }
    if (input.isMouseButtonPressed(sf::Mouse::XButton1)) {
        mouseTimelinePosition = flooredModulo(static_cast<int>(input.getMousePosition().x*timelineLength/static_cast<double>(mouseXOfEndOfTimeline)),timelineLength);
    }
    if (input.isMouseButtonPressed(sf::Mouse::XButton2)) {
        mouseTimelinePosition = flooredModulo(static_cast<int>(input.getMousePosition().x*timelineLength/static_cast<double>(mouseXOfEndOfTimeline)),timelineLength);
    }
    mouseX = static_cast<int>(std::round(input.getMousePosition().x*mouseScale));
    mouseY = static_cast<int>(std::round(input.getMousePosition().y*mouseScale));
}

InputList Input::AsInputList() const
{
    return InputList(
        GuyInput(
            left,
            right,
            up,
            down == 1,
            space == 1,     //portalUsed
            mouseLeft == 1, //abilityUsed
            abilityCursor,
            FrameID(mouseTimelinePosition, UniverseID(timelineLength)),     
            mouseX,
            mouseY),
        0/*TODO: allow overriding of past inputs*/);
}
}

