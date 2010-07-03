/*
 *  main.cpp
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 30/06/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */
#include <allegro.h>

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
        printf("y1");
        //thread test
        boost::thread* forever = new boost::thread(main_);
       // Init(); - loads program options, 
        //make new allegro thread - performs ALL allegro calls
        
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
