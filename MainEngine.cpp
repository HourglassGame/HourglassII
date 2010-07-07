/*
 *  MainEngine.cpp
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 5/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */

#include "MainEngine.h"
#include <allegro.h>
#include "Logger.h"
#include "EngineShutDownException.h"
namespace hg {
    void RunEngine(EngineThreadMediator& mediator)
    {
        HG_TRACE_FUNCTION
        try {
            for (;;) {
                mediator.RunNextAction();
            }
        }
        catch (EngineShutDownException&) {
            Logger::GetLogger().Log("The Main thread has left the mediator",loglevel::INFO);
        }
    }
}
