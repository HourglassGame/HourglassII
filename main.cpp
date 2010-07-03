/*
 *  main.cpp
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 30/06/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */
#include <allegro.h>

#include "AllegroEngine.h"
#include <boost/thread.hpp>
#include "Tracer.h"

int main(int argc, const char* const* argv);
int main_();
//Program options (from command line/config file/wherever) 
//and working subsystems are globally stored.
//NOTHING ELSE IS, BEFORE YOU ADD ANY MORE GLOBAL STATE ADD IT TO THIS LIST
    int main(const int argc, const char* const* const argv)
    {
        HG_TRACE_FUNCTION
        boost::thread* forever = new boost::thread(main_);
       // Init();
        //engine.getCapabilites();
        //make new window thread - takes care of all actual drawing,
        //                      asks view-controller for info every 
        //                      vertical-refresh.
        
        //make new view-controller - gets passed 
        //Scene& scene(MAIN_MENU);
        //while (scene != EXIT) {
        //    scene = scene.run();
        //};
        return 0;
    }
    END_OF_MAIN()
int main_()
{
    for (;;) {
        printf("y0");
    }
    return 0;
}
namespace hg {
}
