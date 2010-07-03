/*
 *  AllegroEngine.cpp
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 1/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */
/*
#include "AllegroEngine.h"
#include <boost/scope_exit.hpp>

using namespace hg;


AllegroEngine::AllegroEngine(int flags) :
capabilites(0)
{
    hg_assert(!instanceExists);
    instanceExists = true;
    bool succeeded = false;
    if(install_allegro(SYSTEM_AUTODETECT, &errno, NULL) == 0) {
        
    }
    else {
        throw std::runtime_exception("install_allegro");
    }


    if (flags & engineflag::KEYBOARD) {
        if(install_keyboard() < 0) {
            throw std::runtime_exception("install_keyboard");
        }
    }
    if (flags & engineflag::MOUSE) {
        if(install_mouse() < 0) {
            throw std::runtime_exception("install_mouse");
        }
    }
    if (flags & engineflag::TIMER) {
        if(install_timer() < 0) {
            throw std::runtime_exception("install_timer");
        }
    }
    if (flags & engineflag::JOYSTICK) {
        if(install_joystick(JOY_TYPE_AUTODETECT) != 0) {
            throw std::runtime_exception("install_joystick");
        }
    }
    succeeded = true;
}

AllegroEngine::~AllegroEngine() {
    allegro_exit();
    instanceExists = false;
}
*/

