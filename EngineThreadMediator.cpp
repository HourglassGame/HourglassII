/*
 *  EngineThreadMediator.cpp
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 4/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */

#include "EngineThreadMediator.h"
#include "EngineShutDownException.h"
#include "DeferredCallWrapper.h"
#include <boost/thread/locks.hpp>
using namespace hg;

//-----------------------USER THREAD ACTIONS------------------------------------
//------------------------------------------------------------------------------


//Adds an action to the queue of actions which the main thread must perform
//Wakes up the main thread so it can perform them
//Manages its own synchronisation (thread-safe)
//TODO - Blocks if the owner/controller is lagging behind requests
void EngineThreadMediator::RequestAction(Action* action)
{
    std::auto_ptr<Action> action_(action);
    HG_TRACE_FUNCTION
    boost::unique_lock<boost::mutex> lock(queueLock);
    while (flushingActions) {
        flushingActionsCond.wait(lock);
    }
    callQueue.push_front(action_);
    hasActionsCond.notify_all();
}

//Performs all requests already on the queue,
//and blocks new requests until finished.
void EngineThreadMediator::Flush()
{
    HG_TRACE_FUNCTION
    boost::lock_guard<boost::mutex> lock(queueLock);
    flushingActions = true;
}

boost::any EngineThreadMediator::GetResult(boost::shared_ptr<DeferredCall> function)
{
    HG_TRACE_FUNCTION
    //Here I wrap the wrapper to make it survive the deletion that occurs to RequestAction things
    DeferredCallWrapper* wrapper = new DeferredCallWrapper(function);
    RequestAction(wrapper); //RequestAction eats the wrapper, do not use after this point
    boost::any result(function->GetResult());
    hg_assert(!result.empty());
    return result;
}

//Causes the main thread to break the next time there are no more actions on the queue
void EngineThreadMediator::Shutdown()
{
    HG_TRACE_FUNCTION
    {
        boost::lock_guard<boost::mutex> lock(queueLock);
        shuttingDown = true;
    }
    hasActionsCond.notify_all();
}
//-----------------------MAIN THREAD ACTIONS------------------------------------
//------------------------------------------------------------------------------
void EngineThreadMediator::RunNextAction()
{
    HG_TRACE_FUNCTION
    {
        boost::unique_lock<boost::mutex> lock(queueLock);
        while (callQueue.empty()) {
            if (flushingActions) {
                flushingActions = false;
                Logger::GetLogger().Log("Flushed",loglevel::FINE);
                flushingActionsCond.notify_all();
            }
            if (shuttingDown) {
                throw EngineShutDownException();
            }
            hasActionsCond.wait(lock);
        }
    }
    boost::unique_lock<boost::mutex> lock(queueLock);
    //10 is just an arbitrary number where the main thread decides it has a bad backlog
    if (!flushingActions && callQueue.size() > 10) {
        Logger::GetLogger().Log("Flushing",loglevel::FINE);
        flushingActions = true;
        return;
    }
    hg_assert(!callQueue.empty());
    boost::ptr_deque<Action>::auto_type nextAction(callQueue.pop_back());
    if (flushingActions && callQueue.empty()) {
        flushingActions = false;
        Logger::GetLogger().Log("Flushed",loglevel::FINE);
        flushingActionsCond.notify_all();
    }
    (*nextAction).Call();
}

EngineThreadMediator::EngineThreadMediator() :
queueLock(),
flushingActions(false),
shuttingDown(false),
callQueue(),
flushingActionsCond(),
hasActionsCond()
{
}
