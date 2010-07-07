/*
 *  PopupOutlet.cpp
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 4/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */

#include "PopupOutlet.h"
#include <sstream>
#include <string>
#include <allegro.h>
#include <boost/bind.hpp>
#include "EngineThreadMediator.h"
#include <iostream>
using namespace hg;
PopupOutlet::PopupOutlet(EngineThreadMediator& mediator, loglevel::LogLevel level) :
level_(level),
mediator_(mediator)
{
}

void PopupOutlet::Log_(const std::string& message, loglevel::LogLevel importance)
{
    if (importance >= level_) {
        std::stringstream message_;
        message_ << importance << ": " << message << std::endl;
        mediator_.RequestCallableAction(boost::bind<void>(allegro_message,"%s","HI\n"/*message_.str().c_str()*/));
    }
}
