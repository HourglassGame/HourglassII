#ifndef HG_MOUSESTATE_H
#define HG_MOUSESTATE_H
/*
 *  MouseState.h
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 5/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */
namespace hg {
    //Sorry, no mickeys, need to use EngineThreadModerator::GetResult for those
    struct MouseState 
    {
        /*
        MouseState(int mouseX,
                   int mouseY,
                   int mouseZ,
                   int mouseW,
                   int mouseB,
                   int mousePos);
         */
        const int mouse_x;
        const int mouse_y;
        const int mouse_z;
        const int mouse_w;
        const int mouse_b;
        const int mouse_pos;
    };
}
#endif //HG_MOUSESTATE_H
