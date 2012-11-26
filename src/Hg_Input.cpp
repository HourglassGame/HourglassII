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
        q(0),
        w(0),
        down(0),
        space(0),
        mouseLeft(0),
        mouseRight(0),
        mouseX(0),
        mouseY(0),
        timelineLength(0)
{
}

void Input::updateState(const sf::Input& input, int mouseXOfEndOfTimeline, double mouseScale)
{
    left = input.IsKeyDown(sf::Key::Left);
    right = input.IsKeyDown(sf::Key::Right);
    up = input.IsKeyDown(sf::Key::Up);
    updatePress(q, input.IsKeyDown(sf::Key::Q));
    updatePress(w, input.IsKeyDown(sf::Key::W));
    updatePress(down, input.IsKeyDown(sf::Key::Down));
    updatePress(space, input.IsKeyDown(sf::Key::Space));
    updatePress(mouseLeft, input.IsMouseButtonDown(sf::Mouse::Left));
    updatePress(mouseRight, input.IsMouseButtonDown(sf::Mouse::Right));
    mouseX = input.GetMouseX()*mouseScale;
    mouseY = input.GetMouseY()*mouseScale;
    mouseTimelinePosition = static_cast<int>(input.GetMouseX() * timelineLength / static_cast<double>(mouseXOfEndOfTimeline));
}

InputList Input::AsInputList() const
{
    if (mouseLeft == 1)
    {
        return InputList(left, right, up, (down == 1), (space == 1), hg::TIME_JUMP, FrameID(mouseTimelinePosition, UniverseID(timelineLength)), 0, 0);
    }

    if (mouseRight == 1)
    {
        return InputList(
            left, right, up, (down == 1), (space == 1),
            hg::TIME_GUN, FrameID(mouseTimelinePosition, UniverseID(timelineLength)), mouseX, mouseY);
    }

    if (q == 1) {
        return InputList(left, right, up, (down == 1), (space == 1), hg::TIME_PAUSE, FrameID(), 0, 0);
    }

    if (w == 1) {
		return InputList(left, right, up, (down == 1), (space == 1), hg::TIME_REVERSE, FrameID(), 0, 0);
    }

    return InputList(left, right, up, (down == 1), (space == 1), hg::NO_ABILITY, FrameID(), 0, 0);
}
}

