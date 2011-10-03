#include "Hg_Input.h"
#include "InputList.h"
namespace hg
{
static void updatePress(int &var, bool inputState)
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
        left(false),
        right(false),
        up(false),
        down(0),
        space(0),
        mouseLeft(0),
        mouseRight(false),
        mouseX(0),
        mouseY(0),
        timelineLength(0)
{
}

void Input::updateState(const sf::Input& input, int mouseXOfEndOfTimeline)
{
    left = input.IsKeyDown(sf::Key::Left);
    right = input.IsKeyDown(sf::Key::Right);
    up = input.IsKeyDown(sf::Key::Up);
    updatePress(down, input.IsKeyDown(sf::Key::Down));
    updatePress(space, input.IsKeyDown(sf::Key::Space));
    updatePress(mouseLeft, input.IsMouseButtonDown(sf::Mouse::Left));
    updatePress(mouseRight, input.IsMouseButtonDown(sf::Mouse::Right));
    mouseX = input.GetMouseX()*100;
    mouseY = input.GetMouseY()*100;
    mouseTimelinePosition = input.GetMouseX() * timelineLength / static_cast<double>(mouseXOfEndOfTimeline);
}

InputList Input::AsInputList() const
{
    if (mouseLeft == 1)
    {
        return InputList(left, right, up, (down == 1) , (space == 1), hg::TIME_JUMP, FrameID(mouseTimelinePosition, UniverseID(timelineLength)), 1);
    }

    if (mouseRight == 1)
    {
        return InputList(left, right, up, (down == 1), (space == 1), hg::TIME_REVERSE, FrameID(), 0);
    }

    return InputList(left, right, up, (down == 1), (space == 1), hg::NO_ABILITY, FrameID(), 0);
}
}

