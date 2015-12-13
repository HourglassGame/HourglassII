//NOT YET IMPLEMENTED!
//This file contains test cases intended to ensure that every level in the
//game is playable.

//The way it works currently is make sure that every `.lua` file in the levels folder,
//has a corresponding `win.replay` file, and that the corresponding `win.replay` file
//results in the level being won.
#include "TestDriver.h"
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include "OperationInterrupter.h"
#include "Level.h"
#include "LevelLoader.h"
#include "ReplayIO.h"
#include "TimeEngine.h"
#include "PlayerVictoryException.h"
#include "prettyprint.hpp"
#include "LuaError.h"
#include <chrono>
#include <iostream>
namespace hg {
namespace levels_test {
namespace {
bool testLevelsLoad() {
    //Iterate the `levels` folder.
    //For every *.lvl that does not
    //contain a file called "DoNotTest",
    //load the level and continue.
    constexpr bool test_levels_load{ false };
    if (!test_levels_load) return true;
    //TODO -- make this test only run in an "expensive tests" run (rather than hardcoding 'test_levels_load = false')
    //TODO -- get rid of hard-coded progress display.
    bool testPassed = true;
    for (auto const entry : boost::make_iterator_range(boost::filesystem::directory_iterator("levels/"),
        boost::filesystem::directory_iterator()))
    {
        if (is_directory(entry.status())
         && entry.path().extension() == ".lvl"
         && !exists(entry.path() / "DoNotTest"))
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
            auto timeTaken =
                std::chrono::duration_cast<std::chrono::duration<double>>(
                    std::chrono::high_resolution_clock::now() - start);
            std::cout << " Loaded OK, in: " << timeTaken.count() << "s\n" << std::flush;
        }
    }
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

    //TODO -- make this test only run in an "expensive tests" run
    //TODO -- get rid of hard-coded progress display.
    bool testPassed = true;
    for (auto const entry : boost::make_iterator_range(boost::filesystem::directory_iterator("levels/"),
        boost::filesystem::directory_iterator()))
    {
        ;
        if (!is_directory(entry.status()) || entry.path().extension() != ".lvl" || exists(entry.path() / "DoNotTest")) continue;

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
                    timeEngine.runToNextPlayerFrame(input);
                }
                if (rewrite_replay) {
                    std::cout << " Extending replay ..." << std::flush;
                    while (true) {
                        outReplay.push_back(InputList());
                        timeEngine.runToNextPlayerFrame(outReplay.back());
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
