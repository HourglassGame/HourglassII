#ifndef HG_LOGGER_H
#define HG_LOGGER_H
/*
 *  Logger.h
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 1/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */
namespace hg {
    class Logger;
}

//Singleton logger
#include <memory>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "HourglassAssert.h"
#include "Outlet.h"
#include "LogLevel.h"

namespace hg {
    class Logger : private boost::noncopyable
    {
    public:
        static Logger& GetLogger();
        void Log(const std::string& message, loglevel::LogLevel importance);
        void RegisterOutlet(Outlet* outlet);
    private:
        Logger();
        ~Logger();

        boost::ptr_vector<Outlet> outlets;
        boost::mutex containerLock;
    };
}
#endif //HG_LOGGER_H
