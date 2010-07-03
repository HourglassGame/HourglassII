#ifndef HG_OUTLET_H
#define HG_OUTLET_H
/*
 *  Outlet.h
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 1/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */
namespace hg {
    class Outlet;
    Outlet* new_clone(const Outlet& o);
}

#include "Logger.h"
#include "HourglassAssert.h"

namespace hg {
    //base class for things which can be logged to.
    class Outlet {
    public:
        virtual ~Outlet() = 0;
        void Log(const std::string& message, loglevel::LogLevel importance);
    private:
        virtual void Log_(const std::string& message, loglevel::LogLevel importance) = 0;
    };
}
#endif //HG_OUTLET_H
