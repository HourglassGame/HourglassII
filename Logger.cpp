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

#define foreach BOOST_FOREACH
using namespace hg;

Logger& Logger::GetLogger()
{
    static Logger instance;
    return instance;
}

//Very simplistic design & implementation, could be made faster and more powerful.
//If you want to do work on it have a look at `Logger' in java and copy. :-P
void Logger::Log(const std::string& message, loglevel::LogLevel importance)
{
    //overly conservative lock, could be avoided by locking each 
    //Outlet individually, and allowing simultaneous calls to this function
    //but not allowing calls to RegisterOutlet() to be simultaneous with either
    //Log or other RegisterOutlet calls.
    boost::lock_guard<boost::mutex> lock(containerLock);
    
    foreach(Outlet& out, outlets) {
        out.Log(message, importance);
    }
}

void Logger::RegisterOutlet(std::auto_ptr<Outlet> outlet)
{
    boost::lock_guard<boost::mutex> lock(containerLock);
    outlets.push_back(outlet);
}

Logger::Logger() :
outlets(boost::ptr_vector<Outlet>())
{
}

//Possible error if destructor called simultaneously with any other function, 
//not sure how to avoid. Needs fixing, but shouldn't 
//be a problem as instance never be deleted except at the end of the program anyway.
Logger::~Logger()
{
}
