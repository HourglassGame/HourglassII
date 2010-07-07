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
#include <boost/ptr_container/ptr_deque.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include "Tracer.h"
#include "Logger.h"
#include "Action.h"
#include "ActionFromCallable.h"
#include "DeferredCall.h"
#include "DeferredCallFromCallable.h"

#include <boost/noncopyable.hpp>
namespace hg {
    class EngineThreadMediator : private boost::noncopyable
    {
    public:
//-----------------------USER THREAD ACTIONS------------------------------------
//------------------------------------------------------------------------------
 
        template<class Callable>
        inline void RequestCallableAction(Callable func)
        {
            HG_TRACE_FUNCTION
            return RequestAction
            (
                new hg::ActionFromCallable
                <
                    Callable
                >
                (
                    (func)
                )
            );
        }
        
        template<class ReturnType,class Callable>
        inline ReturnType GetCallableResult(Callable func)
        {
            //HG_TRACE_FUNCTION here causes a deadlock!
            //HG_TRACE_FUNCTION
            return boost::any_cast<ReturnType>
            (
                GetResult
                (
                 //Here I wrap the Callable to make it useable in my code (ie be an appropriate subclass)
                 //OK to take address here as GetResult blocks this thread until it no longer needs the DeferredFunctionCall
                    &hg::DeferredCallFromCallable
                    <
                        Callable
                    >
                    (
                        func
                    )
                )
            );
        }
        
        
        //Adds an action to the queue of actions which the main thread must perform
        //Wakes up the main thread so it can perform them
        //Manages its own synchronisation (thread-safe)
        //Usually you would give a function object to RequestCallableAction,
        //but if you want to subclass VoidDeferredCall yourself then you can use this
        void RequestAction(Action* action);
        
        //Performs all requests already on the queue,
        //and blocks new requests until finished.
        void Flush();
        
        //Takes a no-args Callable type,
        //blocks until the main function can be bothered running the callable and then returns the result
        //There is very probably a smart way to do this with templates
        //Gotta make sure that the callable dosen't take too long (or get stuck in an infinite loop)
        //as it takes control of the main thread
        //Usually you would give a function object to GetCallableResult,
        //but if you want to subclass DeferredCall yourself then you can use this
        boost::any GetResult(DeferredCall* function);

        //Causes the main thread to break the next time there are no more actions on the queue
        void Shutdown();
//-----------------------MAIN THREAD ACTIONS------------------------------------
//------------------------------------------------------------------------------
        void RunNextAction();
        
        EngineThreadMediator();
        
    private:
        boost::mutex queueLock;
        bool flushingActions;
        bool shutingDown;
        
        boost::ptr_deque<Action> callQueue;
        boost::condition_variable flushingActionsCond;
        boost::condition_variable hasActionsCond;
    };
}
#endif //HG_ENGINETHREADMEDIATOR_H
