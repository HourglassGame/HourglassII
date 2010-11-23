#include "Hg_Input.h"
#include <SFML/Window/Input.hpp>
#include <iostream>
#include <cassert>

namespace hg {
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
mouseY(0),
pause(0)
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
    updatePress(pause, input.IsKeyDown(sf::Key::P));
}

const InputList Input::AsInputList() const
{
    if (mouseLeft == 1)
    {
        return InputList(left, right, up, (down == 1) , space, hg::TIME_JUMP, NewFrameID(mouseX*10800/64000, 10800), 1);
    }

    if (mouseRight == 1)
    {
        return InputList(left, right, up, (down == 1), space, hg::TIME_REVERSE, NewFrameID(), 0);
    }

    if (pause == 1)
    {
        return InputList(left, right, up, (down == 1), space, hg::PAUSE_TIME, NewFrameID(), 0);
    }

     return InputList(left, right, up, (down == 1), space, hg::NO_ABILITY, NewFrameID(), 0);
}
}
