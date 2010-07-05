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
    inline hg::Outlet* new_clone(const hg::Outlet&);
}

#include "Logger.h"
#include "LogLevel.h"
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
    inline hg::Outlet* new_clone(const hg::Outlet&)
    {
        hg_assert(false && "Trying to clone an Outlet- which is non-cloneable");
        return NULL;
    }
}

#endif //HG_OUTLET_H
