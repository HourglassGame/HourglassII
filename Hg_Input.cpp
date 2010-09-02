#include <iostream>
#include <cassert>

#include "Hg_Input.h"

using namespace hg;

void updatePress(int &var, bool inputState)
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

Input::Input(const sf::Input& input) :
left(input.IsKeyDown(sf::Key::Left)),
right(input.IsKeyDown(sf::Key::Right)),
up(input.IsKeyDown(sf::Key::Up)),
space(input.IsKeyDown(sf::Key::Space)),
mouseRight(input.IsMouseButtonDown(sf::Mouse::Right)),
mouseX(input.GetMouseX()*100),
mouseY(input.GetMouseY()*100)
{
    updatePress(mouseLeft, input.IsMouseButtonDown(sf::Mouse::Left));
    updatePress(down, input.IsKeyDown(sf::Key::Down));
}

const InputList Input::AsInputList() const
{
    if (mouseLeft == 1 or mouseLeft == -1)
    {
        return InputList(left, right, up, (down == 1), space, hg::TIME_JUMP, FrameID(mouseX*5400/64000), 1);
    }
   else
   {
        return InputList(left, right, up, (down == 1), space, hg::NO_ABILITY, 0, 0);
   }
}
