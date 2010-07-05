#ifndef HG_JOYSTICKSTATE_H
#define HG_JOYSTICKSTATE_H
/*
 *  JoystickState.h
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 5/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */
#include <allegro.h>
namespace hg {
//See allegro ducumentation
    //meh, this probably dosen't work, but we're not using joysticks for anything anyway. 
struct JoystickState 
{
    JOYSTICK_INFO joy[8];
};
}

#endif //HG_JOYSTICKSTATE_H
