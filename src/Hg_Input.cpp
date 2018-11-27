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
    boxLeft(),
    boxRight(),
    space(),
    mouseLeft(),
    abilityCursor(),
    mouseTimelinePosition(),
    mousePersonalTimelinePosition(),
    mouseX(),
    mouseY(),
    timelineLength()
{
}

void Input::updateState(hg::RenderWindow::InputState const &input, ActivePanel const mousePanel,
    int mouseXTimelineOffset, int mouseXOfEndOfTimeline, int mouseXOfEndOfPersonalTimeline, std::size_t personalTimelineLength,
    int mouseOffX, int mouseOffY, double mouseScale)
{
    left = input.isKeyPressed(sf::Keyboard::A);
    right = input.isKeyPressed(sf::Keyboard::D);
    up = input.isKeyPressed(sf::Keyboard::W);
    updatePress(down, input.isKeyPressed(sf::Keyboard::S));
    updatePress(boxLeft, input.isKeyPressed(sf::Keyboard::Q));
    updatePress(boxRight, input.isKeyPressed(sf::Keyboard::E));
    updatePress(space, input.isKeyPressed(sf::Keyboard::Space));
    
    updatePress(mouseLeft, input.isMouseButtonPressed(sf::Mouse::Left) && mousePanel == ActivePanel::WORLD);

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

    if (input.isMouseButtonPressed(sf::Mouse::Left) && mousePanel == ActivePanel::PERSONAL_TIME) {
        int mousePosition = std::max(0, std::min(mouseXOfEndOfPersonalTimeline, input.getMousePosition().x - mouseXTimelineOffset));
        mousePersonalTimelinePosition = static_cast<int>(personalTimelineLength - (static_cast<int>(mousePosition*personalTimelineLength / static_cast<double>(mouseXOfEndOfPersonalTimeline))));
    }
    if ((input.isMouseButtonPressed(sf::Mouse::Left) && mousePanel == ActivePanel::GLOBAL_TIME) ||
        input.isMouseButtonPressed(sf::Mouse::Right) || 
        input.isMouseButtonPressed(sf::Mouse::Middle) || 
        input.isMouseButtonPressed(sf::Mouse::XButton1) || 
        input.isMouseButtonPressed(sf::Mouse::XButton2)) 
    {
        int mousePosition = std::max(0, std::min(mouseXOfEndOfTimeline - 1, input.getMousePosition().x - mouseXTimelineOffset));
        mouseTimelinePosition = static_cast<int>(mousePosition*timelineLength/static_cast<double>(mouseXOfEndOfTimeline));
    }
    mouseX = static_cast<int>(std::round((input.getMousePosition().x - mouseOffX)*mouseScale));
    mouseY = static_cast<int>(std::round((input.getMousePosition().y - mouseOffY)*mouseScale));
}

InputList Input::AsInputList() const
{
    return InputList(
        GuyInput(
            left,
            right,
            up,
            down == 1,
            boxLeft == 1,
            boxRight == 1,
            space == 1,     //portalUsed
            mouseLeft == 1, //abilityUsed
            abilityCursor,
            FrameID(mouseTimelinePosition, UniverseID(timelineLength)),     
            mouseX,
            mouseY),
        mousePersonalTimelinePosition);
}
}

