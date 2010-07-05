#ifndef HG_ENGINETHREADMEDIATOR_H
#define HG_ENGINETHREADMEDIATOR_H
/*
 *  EngineThreadMediator.h
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 4/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */

#include <boost/any.hpp>
#include "Tracer.h"
#include "KeyboardState.h"
#include "MouseState.h"
#include "Action.h"
namespace hg {
    class EngineThreadMediator 
    {
    public:
        //User functions
        //returns by value, use of ExclusiveReference *could* be faster (especially if used with shared view reference)
        KeyboardState GetKeyboardState();
        MouseState GetMouseState();
        //Blocks if the owner/controller is lagging behind requests
        void RequestAction(std::auto_ptr<Action> action)
        {
        }
        
        //Performs all requests already stacked up, blocks new requests until finished.
        void Flush(){}
        
        //Takes a no-args Callable type,
        //blocks until the main function can be bothered running the callable and then returns the result
        //There is very probably a smart way to do this with templates
        //Gotta make sure that the callable dosen't take too long (or get stuck in an infinite loop)
        //as it takes control of the main thread
        //Ideally this would return the right type, rather than a boost::any, but I can't get that to work
        //The way this works means that you really want to only use small types (pointers and such)
        //The penalty for copying objects is even worse than usual (I think)
        template<class Callable>
        boost::any GetResult(Callable function)
        {
            HG_TRACE_FUNCTION
            //Just a test atm, not going to do this:
            return boost::any(function());
        }

    private:
        //Owner functions (main thread functions)
        void InteruptAllThreads();
    };
}
#endif //HG_ENGINETHREADMEDIATOR_H
