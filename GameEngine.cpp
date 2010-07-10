/*
 *  GameEngine.cpp
 *  HourglassIIGameAlleg4
 *
 *  Created by Evan Wallace on 5/07/10.
 *  Copyright 2010 Team Causality. All rights reserved.
 *
 */

#include "GameEngine.h"
#include <iostream>
#include <allegro.h>
#include "Logger.h"
#include "TimeEngine.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
namespace hg {
    void DoStep(EngineThreadMediator& mediator,TimeEngine& timeEngine,const std::vector<std::vector<bool> >& wall,const std::vector<char>& keyboardInput);
    std::vector<char> GetKeyboardState();
    std::vector<std::vector<bool> > MakeWall();
    TimeEngine MakeTimeEngine(std::vector<std::vector<bool> > wall);
    void DrawOutput(EngineThreadMediator& mediator, const boost::shared_ptr<ObjectList> output, const std::vector<std::vector<bool> >& wall);
    void DrawWall(EngineThreadMediator& mediator,const std::vector<std::vector<bool> >& wall);
    void DrawObjects(EngineThreadMediator& mediator, boost::shared_ptr<ObjectList> Objects);
    void DrawSwitches(EngineThreadMediator& mediator, vector<boost::shared_ptr<Switch> > switchList);
    void DrawPlatforms(EngineThreadMediator& mediator, vector<boost::shared_ptr<Platform> > platformList);
    void DrawPickups(EngineThreadMediator& mediator, vector<boost::shared_ptr<Pickup> >pickupList);
    void DrawItems(EngineThreadMediator& mediator,vector<boost::shared_ptr<Item> >itemList);
    void DrawBoxes(EngineThreadMediator& mediator,vector<boost::shared_ptr<Box> >boxList);
    void DrawGuys(EngineThreadMediator& mediator,vector<boost::shared_ptr<Guy> >guyList);
    
    
    std::vector<char> GetKeyboardState();
    std::vector<char> GetKeyboardState()
    {
        poll_keyboard();
        std::vector<char> result;
        result.reserve(KEY_MAX);
        for (int i = 0; i < KEY_MAX; ++i) {
            result.push_back(key[i]?true:false);
        }
        return result;
    }
    
    void RunGame(EngineThreadMediator& mediator)
    {
        if(mediator.GetCallableResult(boost::bind<int>(install_allegro,SYSTEM_AUTODETECT, &errno,atexit)) != 0) {
            Logger::GetLogger().Log("Could not initialise Allegro Library",loglevel::FATAL);
        }
        if(mediator.GetCallableResult(&install_keyboard) < 0) {
            Logger::GetLogger().Log("Could not initialise keyboard",loglevel::FATAL);
        }
        if(mediator.GetCallableResult(&install_mouse) < 0) {
            Logger::GetLogger().Log("Could not initialise mouse",loglevel::FATAL);
        }
        if(mediator.GetCallableResult(boost::bind(set_gfx_mode, GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0)) < 0) {
            Logger::GetLogger().Log("Could not initialise graphics mode",loglevel::FATAL);
        }
        std::vector<std::vector<bool> > wall(MakeWall());
        TimeEngine timeEngine(MakeTimeEngine(wall));
        
        printf("hereGGG");
        
        boost::posix_time::time_duration stepTime(0,0,0,boost::posix_time::time_duration::ticks_per_second()/60);
        std::vector<char> keyboardInput(mediator.GetCallableResult(&GetKeyboardState));
        //lol the synchronisation with the other thread happens here, so it can never get out of time.
        while(!(keyboardInput[KEY_ESC])) {
            boost::posix_time::ptime startTime(boost::posix_time::microsec_clock::universal_time());
            
            DoStep(mediator, timeEngine, wall, keyboardInput);
            while (boost::posix_time::microsec_clock::universal_time() - startTime < stepTime) {
                boost::this_thread::sleep(boost::posix_time::milliseconds(1));
            }
            keyboardInput = mediator.GetCallableResult(&GetKeyboardState);
        }
        mediator.Shutdown();
        
        
        //Scene scene(mediator, scene::FIRST_RESPONDER);
        //while (scene != scene::EXIT) {
        //    scene = scene.run(engineAccess);
        //}
        //mediator.Shutdown(); //Could be put into scene(EXIT); ?
        
        //TODO print stack-trace here if needed
    }
    std::vector<std::vector<bool> > MakeWall()
    {
        using namespace boost::assign;
        std::vector<std::vector<bool> > wall;
        std::vector<bool> row;
        printf("hereAAA");
        row += 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1;
        wall.push_back(row);
        row.clear();
        row += 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1;
        wall.push_back(row);
        row.clear();
        row += 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1;
        wall.push_back(row);
        row.clear();
        row += 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1;
        wall.push_back(row);
        row.clear();
        row += 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1;
        wall.push_back(row);
        row.clear();
        row += 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1;
        wall.push_back(row);
        row.clear();
        row += 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1;
        wall.push_back(row);
        row.clear();
        row += 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1;
        wall.push_back(row);
        row.clear();
        row += 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1;
        wall.push_back(row);
        row.clear();
        row += 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1;
        wall.push_back(row);
        row.clear();
        row += 1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1;
        wall.push_back(row);
        row.clear();
        row += 1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1;
        wall.push_back(row);
        row.clear();
        row += 1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1;
        wall.push_back(row);
        row.clear();
        row += 1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1;
        wall.push_back(row);
        row.clear();
        row += 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1;
        wall.push_back(row);
        row.clear();
        std::vector<std::vector<bool> > actualWall;
        actualWall.resize(wall.at(0).size());
        foreach(std::vector<bool>& column, actualWall) {
            column.resize(wall.size());
        }
        for(unsigned int i = 0; i < wall.size(); ++i) {
            for(unsigned int j = 0; j < wall.at(i).size(); ++j) {
                actualWall.at(j).at(i) = wall.at(i).at(j);
            }
        }
        Logger::GetLogger().Log("Made Wall", loglevel::FINE);
        return actualWall;
    }
    TimeEngine MakeTimeEngine(std::vector<std::vector<bool> > wall)
    {
  
        printf("hereBBB");
        TimeEngine newEngine(5400,wall,3200,50);
        printf("hereCCC");
        boost::shared_ptr<ObjectList> newObjectList(new ObjectList());
        newObjectList->addBox(6400, 25600, 0 ,0, 3200, 1);
        //int x, int y, int xspeed, int yspeed, int width, int height, int timeDirection, bool boxCarrying, int boxCarrySize, int boxCarryDirection, int relativeIndex, int subimage
        newObjectList->addGuy(22000, 6400, 0, 0, 1600, 3200, 1, false, 0, 0, 0, 0);
        printf("hereDDD");
        std::cout << newEngine.checkConstistencyAndPropagateLevel(newObjectList,0) << std::endl;
        printf("hereEEE");
        return newEngine;
    }
    
    void DoStep(EngineThreadMediator& mediator,TimeEngine& timeEngine,const std::vector<std::vector<bool> >& wall,const std::vector<char>& keyboardInput)
    {
        //InputList::InputList(bool nLeft, bool nRight, bool nUp, bool nDown, bool nSpace, bool nMouseLeft, bool nMouseRight, int nMouseX, int nMouseY)   
        boost::shared_ptr<InputList> input(new InputList(keyboardInput[KEY_LEFT],keyboardInput[KEY_RIGHT],keyboardInput[KEY_UP],keyboardInput[KEY_DOWN],keyboardInput[KEY_SPACE],false,false,50,50));
        vector<boost::shared_ptr<ObjectList> > output(timeEngine.getNextPlayerFrame(input));
        if (output.size() > 1) {
            printf("Paradox foo");
        }
        DrawOutput(mediator, output.at(0), wall);
    }
    
    void DrawOutput(EngineThreadMediator& mediator, const boost::shared_ptr<ObjectList> output, const std::vector<std::vector<bool> >& wall)
    {
        DrawWall(mediator, wall);
        DrawObjects(mediator,output);
    }
    void DrawWall(EngineThreadMediator& mediator,const std::vector<std::vector<bool> >& wall)
    {
        mediator.RequestCallableAction(boost::bind<void>(clear_to_color,screen, boost::bind<int>(makecol,255,255,255)));
        for(unsigned int i = 0; i < wall.size(); ++i) {
            for(unsigned int j = 0; j < wall.at(i).size(); ++j) {
                if (wall.at(i).at(j)) {
                    mediator.RequestCallableAction(boost::bind<void>(rectfill,screen,32*i,32*j,32*(i+1),32*(j+1), boost::bind<int>(makecol,0,0,0)));
                }
            }
        }
    }
    void DrawObjects(EngineThreadMediator& mediator, const boost::shared_ptr<ObjectList> objects)
    {
        DrawSwitches( mediator, objects->getSwitchList());
        DrawPlatforms(mediator, objects->getPlatformList());
        DrawPickups(mediator, objects->getPickupList());
        DrawItems(mediator,objects->getItemList());
        DrawBoxes(mediator,objects->getBoxList());
        DrawGuys(mediator,objects->getGuyList());
    }
    void DrawSwitches(EngineThreadMediator& mediator, vector<boost::shared_ptr<Switch> > switchList)
    {
    }
    void DrawPlatforms(EngineThreadMediator& mediator, vector<boost::shared_ptr<Platform> > platformList)
    {
    }
    void DrawPickups(EngineThreadMediator& mediator, vector<boost::shared_ptr<Pickup> >pickupList)
    {
    }
    void DrawItems(EngineThreadMediator& mediator,vector<boost::shared_ptr<Item> >itemList)
    {
    }
    void DrawBoxes(EngineThreadMediator& mediator,vector<boost::shared_ptr<Box> >boxList)
    {
        foreach(boost::shared_ptr<Box>& box, boxList) {
            mediator.RequestCallableAction(boost::bind<void>(rectfill,screen,box->getX()/100,box->getY()/100,
                                                             (box->getX()+ box->getSize())/100,(box->getY()+box->getSize())/100, boost::bind<int>(makecol,150,150,0)));
        }
    }
    void DrawGuys(EngineThreadMediator& mediator,vector<boost::shared_ptr<Guy> >guyList)
    {
        foreach(boost::shared_ptr<Guy>& guy, guyList) {
            mediator.RequestCallableAction(boost::bind<void>(rectfill,screen,guy->getX()/100,guy->getY()/100,
                                                             (guy->getX()+ guy->getWidth())/100,(guy->getY()+guy->getHeight())/100, boost::bind<int>(makecol,150,150,0)));
        }
    }
}
