/*
 *  Logger.cpp
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 1/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */

#include "Logger.h"
#include <boost/foreach.hpp>
#include <boost/thread/locks.hpp>
#include <iostream>
#include "TerminalOutlet.h"
#define foreach BOOST_FOREACH
using namespace hg;

boost::once_flag Logger::init_flag=BOOST_ONCE_INIT;
Logger* Logger::instance;

Logger& Logger::GetLogger()
{
    boost::call_once(init_flag, InitInstance);
    return *instance;
}

void Logger::InitInstance()
{
    instance = new Logger();   
}

//Very simplistic design & implementation, could be made faster and more powerful.
//If you want to do work on it have a look at `Logger' in java and copy. :-P
void Logger::Log(const std::string& message, loglevel::LogLevel importance)
{
    //overly conservative lock, could be avoided by locking each 
    //Outlet individually, and allowing simultaneous calls to this function
    //but not allowing calls to `RegisterOutlet' to be simultaneous with either
    //`Log' or `RegisterOutlet' calls.
    boost::lock_guard<boost::mutex> lock(containerLock);
    foreach(Outlet& out, outlets) {
        out.Log(message, importance);
    }
}

void Logger::RegisterOutlet(Outlet* outlet)
{
    std::auto_ptr<Outlet> outlet_(outlet);
    boost::lock_guard<boost::mutex> lock(containerLock);
    
    static bool firstCall = true;
    //Remove the default Outlet once another outlet has been specified
    if (firstCall) {
        outlets.clear();
        firstCall = false;
    }
    
    outlets.push_back(outlet_);
}

Logger::Logger() :
outlets(boost::ptr_vector<Outlet>()),
containerLock()
{
    outlets.push_back(new TerminalOutlet(loglevel::ALL));
}

//Possible error if destructor called simultaneously with any other function, 
//not sure how to avoid. Needs fixing, but shouldn't 
//be a problem as instance never be deleted except at the end of the program anyway.
Logger::~Logger()
{
}
