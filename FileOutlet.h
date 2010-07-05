#ifndef HG_FILEOUTLET_H
#define HG_FILEOUTLET_H
/*
 *  FileOutlet.h
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 4/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */
namespace hg {
    class FileOutlet;
}
#include "Outlet.h"
#include <string>
#include <boost/noncopyable.hpp>
namespace hg {
    class FileOutlet : public Outlet, private boost::noncopyable
    {
    public:
        FileOutlet(loglevel::LogLevel level, const std::string& fileName);
    private:
        void Log_(const std::string& message, loglevel::LogLevel importance);
        //stores the minimum LogLevel of message which the TerminalOutlet will output
        loglevel::LogLevel level_;
        std::string fileName_;
        //Helper for thread-saftey
        boost::mutex serialiser;
    };
}
#endif //HG_FILEOUTLET_H
