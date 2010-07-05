/*
 *  KeyboardState.cpp
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 5/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */

#include "KeyboardState.h"
#include <allegro.h>
using namespace hg;
KeyboardState::KeyboardState(const char* keys, int keyShifts) :
key_shifts(keyShifts),
key()
{
    key.reserve(KEY_MAX);
    for (int i = 0; i < KEY_MAX; ++i) {
        key.push_back(keys[i]);
    }
}
