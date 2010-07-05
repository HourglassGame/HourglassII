/*
 *  main.cpp
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 30/06/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */
#include <allegro.h>
#include <iostream>

#include <boost/thread.hpp>
#include "EngineThreadMediator.h"
#include "Tracer.h"
#include <boost/any.hpp>
#include <boost/bind.hpp>
#include "Exception.h"
#include "Logger.h"
#include "FileOutlet.h"
#include "PopupOutlet.h"
#include "LogLevel.h"
#include "HourglassAssert.h"
#include "MainEngine.h"
#include "GameEngine.h"

int main(int argc, const char* const* argv);
namespace hg {
    void LoadProgramOptions(int argc, const char* const* argv);
    void StartLogger(EngineThreadMediator& engine);

}
//Program options (from command line/config file/wherever) are globally stored.
//NOTHING ELSE IS, BEFORE YOU ADD ANY MORE GLOBAL STATE ADD IT TO THIS LIST
int main(const int argc, const char* const* const argv)
{
    HG_TRACE_FUNCTION
    try {
        hg::LoadProgramOptions(argc,argv);//Loads program options
        hg::EngineThreadMediator mediator;
        hg::StartLogger(mediator); //registers outlets with logger (all earlier logging is done to stdout)
        
        //Game Thread
        boost::thread game(boost::bind(boost::type<void>(),hg::RunGame,
                                             boost::ref(mediator)));
        hg::Logger::GetLogger().Log("h00000", hg::loglevel::SEVERE);
        //performs ALL allegro calls in main thread (may use helper-threads which don't directly call allegro)
        hg::RunEngine(mediator);
        
        //Perhaps don't have this, dunno? (wait for impl)
        game.interrupt();
    }
    catch (hg::Exception& e) {
        hg::Logger::GetLogger().Log(hg::Tracer::GetStringBackTrace(), hg::loglevel::SEVERE);
    }
    catch (std::exception& e) {
        hg::Logger::GetLogger().Log(hg::Tracer::GetStringBackTrace(), hg::loglevel::SEVERE);
    }
    catch (...) {
        hg::Logger::GetLogger().Log(hg::Tracer::GetStringBackTrace(), hg::loglevel::SEVERE);
    }
    
    return 0;
}
END_OF_MAIN()

namespace hg {
    //TODO- Does nothing yet, want to get a prototype working first
    void LoadProgramOptions(int argc, const char* const* argv)
    {
        HG_TRACE_FUNCTION
    }
    
    //Registers a Popup outlet at log level FATAL 
    //and a File outlet at log level INFO
    //In final version this would be specified by program options
    void StartLogger(EngineThreadMediator& engine)
    {
        HG_TRACE_FUNCTION
        Logger::GetLogger().RegisterOutlet(new FileOutlet(loglevel::INFO, std::string("./log.txt")));
        Logger::GetLogger().RegisterOutlet(new PopupOutlet(engine, loglevel::FATAL));
    }
}
