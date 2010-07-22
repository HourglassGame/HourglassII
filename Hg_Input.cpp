/*
 *  Hg_Input.cpp
 *  HourglassIIGameSFML
 *
 *  Created by Evan Wallace on 20/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

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

boost::shared_ptr<InputList> Input::AsInputList() const
{
    return boost::shared_ptr<InputList>(new InputList(left, right, up, down, space, mouseLeft, mouseRight, mouseX, mouseY));
}
