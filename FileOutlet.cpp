/*
 *  FileOutlet.cpp
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 4/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */

#include "FileOutlet.h"
#include <fstream>
using namespace hg;

FileOutlet::FileOutlet(loglevel::LogLevel level, const std::string& fileName) :
level_(level),
fileName_(fileName),
serialiser()
{
}

//TODO - consider exception safety (need internet to research these functions );)
//TODO - better system for file logging (atm it continually appends, leading to confusion about 
//which message is from which session and potential for infinite log size after a while)
void FileOutlet::Log_(const std::string& message, const loglevel::LogLevel importance)
{
    if (importance >= level_) {
        boost::lock_guard<boost::mutex> lock(serialiser);
        std::ofstream file(fileName_.c_str(), std::ios::out | std::ios::app);
        file << importance << ": " << message << std::endl;
        //TODO - file stays open if ^ throws.
        file.close();
    }
}
