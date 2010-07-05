/*
 *  TerminalOutlet.cpp
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 4/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */

#include "TerminalOutlet.h"
#include <boost/thread/locks.hpp>
#include <iostream>
using namespace hg;

TerminalOutlet::TerminalOutlet(const loglevel::LogLevel level) :
level_(level),
serialiser()
{
}

void TerminalOutlet::Log_(const std::string& message, const loglevel::LogLevel importance)
{
    if (importance >= level_) {
        boost::lock_guard<boost::mutex> lock(serialiser);
        std::cout << importance << ": " << message << std::endl;
    }
}
