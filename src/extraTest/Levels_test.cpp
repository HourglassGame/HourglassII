//This file contains test cases intended to ensure that every level in the
//game is playable.

//The way it works currently is make sure that every `.lua` file in the levels folder,
//has a corresponding `win.replay` file, and that the corresponding `win.replay` file
//results in the level being won.
#include "hg/Util/TestDriver.h"
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include "hg/Util/OperationInterrupter.h"
#include "hg/TimeEngine/Level.h"
#include "hg/TimeEngine/LevelLoader.h"
#include "hg/FrontEnd/ReplayIO.h"
#include "hg/TimeEngine/TimeEngine.h"
#include "hg/TimeEngine/PlayerVictoryException.h"
#include "hg/Util/prettyprint.hpp"
#include "hg/LuaUtil/LuaError.h"
#include <chrono>
#include <iostream>
#include <set>
namespace hg {
namespace levels_test {
namespace {
#if 0
std::set<std::string> const levelsToTest{
    "1EasyStart.lvl",
    "2OpenAndClosed.lvl",
    "3StandardBoxPuzzle.lvl",
    "4NotSoStandard.lvl",
    //"5GoingUp.lvl",//Needs toggleSwitch
    "6StackedHigh.lvl",
    "7Timebelt.lvl",
    "8AllTheWayUp.lvl",
    //"9OneWayTrip.lvl",//Needs stickyLaserSwitch
    "13FishInABarrel.lvl",
    //"17ItsATrap.lvl",//Needs stickyLaserSwitch
    "19a_Really_Impossibru.lvl",
    "19Impossibru.lvl",
    //"20WhatGoesDown.lvl",//Needs triggerFunction support in collisions
    "21WhichWay.lvl",
    "22AnotherBoxPuzzle.lvl",
    "25Shafted.lvl",
    //"27WhichWayII.lvl",//Needs triggerFunction support in collisions
    "31UpsAndDowns.lvl",
    "33Upsypupsy.lvl",
    "38SomethingMissing.lvl",
    //"39TwoBoxes.lvl",//TODO
    "40TooHigh.lvl",
    "44Widdershins.lvl",
    "DoorNumberThree.lvl",
    "Elevator.lvl",
    //"GoneWithTheTimeWave.lvl",//Needs advanced scripting
    //"GoneWithTheTimeWave2.lvl",//Needs advanced scripting
    //"HoldThemAll.lvl",//needs multistickyswitch
    //"LowProfile.lvl",//Needs advanced scripting
    //"Ontological.lvl",//Needs advanced scripting
    "OppositesAnnihilate.lvl",
    "ReverseBootstrap.lvl",

    //"Splitters.lvl",//Needs multiStickySwitch, and advanced scripting for glitz
    "StarterEchoBox.lvl",
    "TestManyBoxes.lvl",
    //"ThePit.lvl",//Needs triggerFunction support in collisions
    "ThinkingWithPortals.lvl",
    //"ThreeSteps.lvl",//Needs triggerFunction support in collisions
    //"WackyTomb.lvl",//Needs triggerFunction support in collisions
    "WaitAMoment.lvl"
};
#endif
bool testLevelsLoad() {

    auto const testStart = std::chrono::high_resolution_clock::now();
    //Iterate the `levels` folder.
    //For every *.lvl that does not
    //contain a file called "DoNotTest",
    //load the level and continue.
    constexpr bool test_levels_load{ false };
    if (!test_levels_load) return true;
    //TODO -- make this test only run in an "expensive tests" run (rather than hardcoding 'test_levels_load = false')
    //TODO -- get rid of hard-coded progress display.
    bool testPassed = true;
    for (auto const& entry : boost::make_iterator_range(boost::filesystem::directory_iterator("levels/"),
        boost::filesystem::directory_iterator()))
    {
        if (is_directory(entry.status())
         && entry.path().extension() == ".lvl"
         //&& !exists(entry.path() / "DoNotTest")
         /*&& levelsToTest.find(entry.path().filename().string()) != levelsToTest.end()*/)
        {
            std::cout << "Test-loading " << entry.path() << " ..." << std::flush;
            auto const start = std::chrono::high_resolution_clock::now();
            try {
                TimeEngine timeEngine = TimeEngine(loadLevelFromFile(entry.path()));
            }
            catch (LuaError const &e) {
                std::string const diagInfo = boost::diagnostic_information(e);
                std::cout << "Failed with: " << diagInfo << "\n" << std::flush;
                testPassed = false;
            }
            auto const timeTaken =
                std::chrono::duration_cast<std::chrono::duration<double>>(
                    std::chrono::high_resolution_clock::now() - start);
            std::cout << " Loaded OK, in: " << timeTaken.count() << "s\n" << std::flush;
        }
    }
    auto const totalTimeTaken =
        std::chrono::duration_cast<std::chrono::duration<double>>(
            std::chrono::high_resolution_clock::now() - testStart);
    std::cout << "Total Loading Time: " << totalTimeTaken.count() << "s\n" << std::flush;
    return testPassed;
}

bool testLevels() {
    bool constexpr test_levels{ false };
    bool constexpr rewrite_replay{ false };

    //usually not run, as these tests take far too long to run to be run at the start of every execution
    if (!test_levels) return true;

    //Iterate the `levels` folder.
    //For every *.lvl that does not
    //contain a file called "DoNotTest",
    //load the level and run the given replay to exhaustion.
    //For the test to pass, every replay must result in
    //a win in the corresponding level.

    auto const testStart = std::chrono::high_resolution_clock::now();
    //TODO -- make this test only run in an "expensive tests" run
    //TODO -- get rid of hard-coded progress display.
    bool testPassed = true;
    for (auto const& entry : boost::make_iterator_range(boost::filesystem::directory_iterator("levels/"),
        boost::filesystem::directory_iterator()))
    {
        ;
        if (!is_directory(entry.status()) || entry.path().extension() != ".lvl"
         || exists(entry.path() / "DoNotTest")
         /*|| levelsToTest.find(entry.path().filename().string()) == levelsToTest.end()*/) continue;

        std::cout << "Testing " << entry.path() << " ..." << std::flush;
        if (!exists(entry.path() / "win.replay")) {
            std::cerr << " Did not have win.replay\n" << std::flush;
            testPassed = false;
            continue;
        }
        auto const start = std::chrono::high_resolution_clock::now();
        try {
            auto timeEngine = TimeEngine(loadLevelFromFile(entry.path()));

            auto const replay = loadReplay((entry.path() / "win.replay").string());
            std::vector<InputList> outReplay;
            try {
                for (auto const& input : replay) {
                    outReplay.push_back(input);
                    timeEngine.runToNextPlayerFrame(input, 0);
                }
                if (rewrite_replay) {
                    std::cout << " Extending replay ..." << std::flush;
                    while (true) {
                        outReplay.push_back(InputList());
                        timeEngine.runToNextPlayerFrame(outReplay.back(), 0);
                    }
                }
            }
            catch (PlayerVictoryException const&) {
                if (rewrite_replay) {
                    if (replay.size() > outReplay.size()) {
                        std::cout << " Truncating replay ..." << std::flush;
                    }
                }
                auto timeTaken =
                    std::chrono::duration_cast<std::chrono::duration<double>>(
                        std::chrono::high_resolution_clock::now() - start);
                std::cout << " OK, time: " << timeTaken.count() << "s, inReplaySize: " << replay.size() << ", outReplaySize: " << outReplay.size() << "\n" << std::flush;
                if (rewrite_replay) {
                    saveReplay((entry.path() / "win.replay").string(), outReplay);
                }
                continue;
            }
        }
        catch (LuaError const &e) {
            std::string const diagInfo = boost::diagnostic_information(e);
            std::cout << "Failed with: " << diagInfo << "\n" << std::flush;
        }
        std::cerr << " Did not win\n" << std::flush;
        testPassed = false;
        continue;
    }

    auto const totalTimeTaken =
        std::chrono::duration_cast<std::chrono::duration<double>>(
            std::chrono::high_resolution_clock::now() - testStart);
    std::cout << "Total Running Time: " << totalTimeTaken.count() << "s\n" << std::flush;
    return testPassed;
}

struct tester {
    tester() {
        ::hg::getTestDriver().registerUnitTest("Levels_testLevelsLoad", testLevelsLoad);
        ::hg::getTestDriver().registerUnitTest("Levels_testLevels", testLevels);
    }
} tester;
}
}
}
