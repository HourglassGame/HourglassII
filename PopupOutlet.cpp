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
#include "Action.h"

using namespace hg;
namespace hg {
    class DoAllegroMessage : public Action
    {
    public:
        DoAllegroMessage(const char* message) :
        message_(message)
        {}
        void operator()()
        {
            allegro_message("%s", message_);
        }
    private:
        const char* message_;
    };
}


PopupOutlet::PopupOutlet(EngineThreadMediator& mediator, loglevel::LogLevel level) :
level_(level),
mediator_(mediator)
{
    
}

void PopupOutlet::Log_(const std::string& message, loglevel::LogLevel importance)
{
    std::stringstream message_;
    message_ << importance << ": " << message << std::endl;
    mediator_.RequestAction(std::auto_ptr<Action>(new DoAllegroMessage(message_.str().c_str())));
}
