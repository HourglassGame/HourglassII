#ifndef HG_TRACER_H
#define HG_TRACER_H

/*
 *  Tracer.h
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 1/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */


#include <deque>
#include <map>
#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>
namespace hg {
    //TODO - design a way to turn this functionality on or off at runtime 
    //(eg through a configuration setting)
    
    //Generates and stores the functions on stack-unwind when an exception is thrown.
    //stores their names as strings in a stack.
    //allows for very useful log/debug info when unexpected exceptions are thrown.
    //To function correctly `HG_TRACE_FUNCTION' must be the first line of every function
    //you wish to have included in the trace 
    //(#defined as `hg::Tracer hg_tracer_object(__func__);')
    
    //It would be easy enough to implement one which stores the actual state of the stack, 
    //but I'm not sure what good that would be.
    
    //THIS CLASS IS ONLY THREAD-SAFE IF THE ONLY THREADS WHICH 
    //CALL IT ARE (`boost::thread')s or the MAIN THREAD
    
    //DO NOT PUT `HG_TRACE_FUNCTION' IN CALLBACK FUNCTIONS FROM EXTERNAL LIBRARIES (eg allegro)
    //AS THIS HAS UNDEFINED BEHAVIOUR DEPENDENT ON THE IMPLEMENTATION OF THE CALLBACKS
    //(it will probably assume the callback functions are at the bottom of the main thread)
    class Tracer : private boost::noncopyable
    {
    public:
        //HG_TRACE_FUNCTION defined here to allow workarounds
        //for the availability of 
        //`__func__' on different compilers to easily be implemented
        //(`__func__' is NOT standard c++ (but is standard c))
        
        //hg_tracer_object is now a pseudo-reserved word, 
        //it uses a different naming convention and a fairly unusual name to 
        //help eliminate any conflicts
#ifdef HG_TRACE_FUNCTION
#error duplicate definition of `HG_TRACE_FUNCTION'
#endif //HG_TRACE_FUNCTION
#define HG_TRACE_FUNCTION hg::Tracer hg_tracer_object(__func__);
        Tracer(const char* functionName);
        
        //Performs magic which creates the back-trace.
        ~Tracer();
        
        //Returns the back-trace for the current thread.
        //Contains nothing unless an exception has been thrown 
        //and the flow of control has not yet left the function in which it is caught,
        //in which case it contains the names (as given by __func__)
        //of the functions in the back-trace which have been captured (using HG_TRACE_FUNCTION)
        //ordered from deepest level function to highest
        static const std::deque<std::string>& GetBackTrace();
    private:
        const char* functionName_;
        
        //same as GetBackTrace() but allows modification.
        static std::deque<std::string>& GetModifiableBackTrace();
        
        //Holds the back-trace
        static std::map<boost::thread::id, std::deque<std::string> >*  backTrace;
        
        //helper function for clean-up
        static void EraseTrace(boost::thread::id whichThread);
        
        static boost::mutex mapLock;
    };
}
#endif //HG_TRACER_H
