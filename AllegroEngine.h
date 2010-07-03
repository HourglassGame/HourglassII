#ifndef HG_ALLEGRO_ENGINE_H
#define HG_ALLEGRO_ENGINE_H
/*
 *  AllegroEngine.h
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 1/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */

#include <allegro.h>
#include <boost/noncopyable.hpp>
/*
namespace hg  {
    class AllegroEngine : public boost::noncopyable {
    public:
        //namespace engineflag {
            enum flag {
                KEYBOARD = 1 << 0,
                MOUSE = 1 << 1,
                SOUND = 1 << 2,
                TIMER = 1 << 3,
                JOYSTICK = 1 << 4
            }
        //}
        
        //Initialises the allegro engine including subsystems denoted by flags.
        //Default initialises all sybsystems.
        //It is illegal to try to have more than one instance at a time.
        explicit AllegroEngine(int flags = engineflag::KEYBOARD|engineflag::MOUSE|engineflag::SOUND|engineflag::TIMER|engineflag::JOYSTICK);
        
        ~AllegroEngine();
        

    private:
        static bool instanceExists = false;
        struct Capabilites {
            bool engineInstalled;
            engineflag::flag subsystems;
            
        }
        Capabilites capabilites;
    };
}
*/
#endif //HG_ALLEGRO_ENGINE_H
