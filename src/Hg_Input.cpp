#include "Hg_Input.h"
#include "InputList.h"
namespace hg
{
static void updatePress(int& var, bool inputState)
{
    if (inputState)
    {
        if (var == 0)
        {
            var = 1;
        }
        else
        {
            var = -1;
        }
    }
    else
    {
        var = 0;
    }
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

void Input::updateState(const sf::Input& input, int mouseXOfEndOfTimeline, double mouseScale)
{
    left = input.IsKeyDown(sf::Key::A);
    right = input.IsKeyDown(sf::Key::D);
    up = input.IsKeyDown(sf::Key::W);
    updatePress(down, input.IsKeyDown(sf::Key::S));
    updatePress(space, input.IsKeyDown(sf::Key::Space));
    
    updatePress(mouseLeft, input.IsMouseButtonDown(sf::Mouse::Left));

    if (input.IsKeyDown(sf::Key::Num1)) {
        abilityCursor = TIME_JUMP;
    }
    if (input.IsKeyDown(sf::Key::Num2)) {
        abilityCursor = TIME_REVERSE;
    }
    if (input.IsKeyDown(sf::Key::Num3)) {
        abilityCursor = TIME_GUN;
    }
    if (input.IsKeyDown(sf::Key::Num4)) {
        abilityCursor = TIME_PAUSE;
    }
    if (input.IsMouseButtonDown(sf::Mouse::Right)) {
        mouseTimelinePosition = static_cast<int>(input.GetMouseX()*timelineLength/static_cast<double>(mouseXOfEndOfTimeline));
    }
    mouseX = input.GetMouseX()*mouseScale;
    mouseY = input.GetMouseY()*mouseScale;
}

InputList Input::AsInputList() const
{
    return InputList(
        left,
        right,
        up,
        down,
        space == 1,     //portalUsed
        mouseLeft == 1, //abilityUsed
        abilityCursor,  
        FrameID(mouseTimelinePosition, UniverseID(timelineLength)),     
        mouseX,
        mouseY);
}
}

