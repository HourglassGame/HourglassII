#include "Hg_Input.h"

using namespace hg;

Input::Input(const sf::Input& input) :
left(input.IsKeyDown(sf::Key::Left)),
right(input.IsKeyDown(sf::Key::Right)),
up(input.IsKeyDown(sf::Key::Up)),
down(input.IsKeyDown(sf::Key::Down)),
space(input.IsKeyDown(sf::Key::Space)),
mouseLeft(input.IsMouseButtonDown(sf::Mouse::Left)),
mouseRight(input.IsMouseButtonDown(sf::Mouse::Right)),
mouseX(input.GetMouseX()*100),
mouseY(input.GetMouseY()*100)
{
}

const InputList Input::AsInputList() const
{
    return InputList(left, right, up, down, space, mouseLeft, mouseRight, mouseX, mouseY);
}
