#ifndef TERMINAL_OUTLET_H
#define TERMINAL_OUTLET_H
/*
 *  TerminalOutlet.h
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 4/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */
namespace hg {
    class TerminalOutlet;
}
#include "Outlet.h"
#include "LogLevel.h"
#include <boost/thread/mutex.hpp>
#include <boost/noncopyable.hpp>
namespace hg {
    class TerminalOutlet : public Outlet, private boost::noncopyable {
    public:
        TerminalOutlet(loglevel::LogLevel level);
    private:
        void Log_(const std::string& message, loglevel::LogLevel importance);
        //stores the minimum LogLevel of message which the TerminalOutlet will output
        loglevel::LogLevel level_;
        //Helper for thread-saftey
        boost::mutex serialiser;
    };
}
#endif //TERMINAL_OUTLET_H
