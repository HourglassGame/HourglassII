/*
 *  Tracer.cpp
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 1/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */

#include "Tracer.h"
#include <boost/thread.hpp>
#include <boost/thread/once.hpp>
#include <boost/bind.hpp>
#include "HourglassAssert.h"
using namespace hg;

std::map<boost::thread::id, std::deque<std::string> >* Tracer::backTrace;
boost::once_flag Tracer::back_init_flag=BOOST_ONCE_INIT;
boost::mutex* Tracer::mapLock;

Tracer::Tracer(const char* const functionName) :
functionName_(functionName)
{
}

//TODO - add code which disables this system and makes a logger message if it runs out of memory
Tracer::~Tracer()
{
    if (std::uncaught_exception()) {
        GetModifiableBackTrace().push_back(functionName_);
    }
    else {
        GetModifiableBackTrace().clear();
    }
}

const std::deque<std::string>& Tracer::GetBackTrace()
{
    return GetModifiableBackTrace();
}

//Perhaps not the fastest or most elegant implementation, 
//but simple enough and gets the job done (I think)
std::deque<std::string>& Tracer::GetModifiableBackTrace()
{
    boost::call_once(InitBackTrace, back_init_flag);
    //Conditionally thread-safe, 
    //as long as the same deque is not returned to two different threads
    //which should never happen, so it's ok.
    boost::lock_guard<boost::mutex> lock(*mapLock);
    //Singleton lazy loading
    /*
    if (backTrace == NULL) {
        backTrace = new std::map<boost::thread::id, std::deque<std::string> >();
    }
    */
    //ensure that map pairs are not kept for dead threads
    
    //mightn't work for the main thread (need to test)
    //but that's hardly a problem because
    //everything will be reclaimed at that point anyway
    
    //There is a hole/bug in the current system which would come about 
    //if a new thread were able to reuse an old thread's id before the old thread's
    //at_thread_exit functions are called
    if (backTrace->find(boost::this_thread::get_id()) == backTrace->end()) {
        boost::this_thread::at_thread_exit(boost::bind<void>(EraseTrace, 
                                                             boost::this_thread::get_id()));
    }

    return (*backTrace)[boost::this_thread::get_id()];
}

//Erases the trace associted with whichThread.
void Tracer::EraseTrace(const boost::thread::id whichThread)
{
    boost::lock_guard<boost::mutex> lock(*mapLock);
    hg_assert(backTrace != NULL);
    backTrace->erase(whichThread);
}

void Tracer::InitBackTrace() {
    mapLock = new boost::mutex();
    backTrace = new std::map<boost::thread::id, std::deque<std::string> >();
}
