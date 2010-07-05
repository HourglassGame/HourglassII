#ifndef HG_LOGLEVEL_H
#define HG_LOGLEVEL_H
/*
 *  LogLevel.h
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 4/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */

#include <climits>
#include "Tracer.h"

namespace hg {
    namespace loglevel {
        enum LogLevel {
            ALL = INT_MIN,
            FINEST = 400,
            FINER = 500,
            FINE = 600,
            CONFIG = 700,
            INFO = 800,
            WARNING = 900,
            SEVERE = 1000,
            FATAL = 1100,
            OFF = INT_MAX
        };
    }
}

template<class charT, class traits>
std::basic_ostream<charT, traits>& operator<< (std::basic_ostream<charT, traits>& os, hg::loglevel::LogLevel const& ll)
{
    HG_TRACE_FUNCTION
    using namespace hg::loglevel;
    switch (ll) {
        case ALL:
            hg_assert(false && "using ALL as a message level");
            break;
        case FINEST:
            return os << "FINEST";
            break;
        case FINER:
            return os << "FINER";  
            break;
        case FINE:
            return os << "FINE";
            break;
        case CONFIG:
            return os << "CONFIG";
            break;
        case INFO:
            return os << "INFO";
            break;
        case WARNING:
            return os << "WARNING";
            break;
        case SEVERE:
            return os << "SEVERE";
            break;
        case FATAL:
            return os << "FATAL";
            break;
        case OFF:
            hg_assert(false && "using OFF as a message level");
            break;
    }
    return os;
}
#endif //HG_LOGLEVEL_H
