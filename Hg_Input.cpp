#include "Hg_Input.h"

#include <iostream>
#include <cassert>

using namespace ::hg;

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
space(false),
mouseLeft(0),
mouseRight(false),
mouseX(0),
mouseY(0)
{
}

void Input::updateState(const sf::Input& input)
{
    left = input.IsKeyDown(sf::Key::Left);
    right = input.IsKeyDown(sf::Key::Right);
    up = input.IsKeyDown(sf::Key::Up);
    updatePress(down, input.IsKeyDown(sf::Key::Down));
    space = input.IsKeyDown(sf::Key::Space);
    updatePress(mouseLeft, input.IsMouseButtonDown(sf::Mouse::Left));
    updatePress(mouseRight, input.IsMouseButtonDown(sf::Mouse::Right));
    mouseX = input.GetMouseX()*100;
    mouseY = input.GetMouseY()*100;
}

const InputList Input::AsInputList() const
{
    if (mouseLeft == 1)
    {
        return InputList(left, right, up, (down == 1), space, hg::TIME_JUMP, FrameID(mouseX*10800/64000), 1);
    }
    else if (mouseRight == 1)
    {
        return InputList(left, right, up, (down == 1), space, hg::TIME_REVERSE, 0, 0);
    }
    else
    {
        return InputList(left, right, up, (down == 1), space, hg::NO_ABILITY, 0, 0);
    }
}
