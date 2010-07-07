/*
 *  main.cpp
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 30/06/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */
#include <allegro.h>

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#include "Logger.h"
#include "Tracer.h"
#include "HourglassAssert.h"

#include "FileOutlet.h"
#include "PopupOutlet.h"
#include "TerminalOutlet.h"

#include "EngineThreadMediator.h"
#include "Exception.h"

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

        //performs ALL allegro calls in main thread (may use helper-threads which don't directly call allegro)
        hg::RunEngine(mediator);

        //Perhaps don't have this, dunno? (wait for impl)
        game.join();
    }
    catch (hg::Exception& e) {
        //TODO print stack-trace here
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
        //Logger::GetLogger().RegisterOutlet(new PopupOutlet(engine, loglevel::FATAL));
        Logger::GetLogger().RegisterOutlet(new TerminalOutlet(loglevel::ALL));
    }
}
