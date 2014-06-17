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
#include <chrono>
#include <iostream>
namespace hg {
namespace levels_test {
namespace {
bool testLevels() {
    //Iterate the `levels` folder.
    //For every *.lvl that does not
    //contain a file called "DoNotTest",
    //load the level and run the given replay to exhaustion.
    //For the test to pass, every replay must result in
    //a win in the corresponding level.

    //TODO -- make this test only run in an "expensive tests" run
    //TODO -- get rid of hard-coded progress display.
    bool testPassed = true;
    for (auto const entry: boost::make_iterator_range(boost::filesystem::directory_iterator("levels/"),
                                                      boost::filesystem::directory_iterator()))
    {
#if 0 //commented out as these tests take far too long to run to be run at the start of every execution
        if (is_directory(entry.status()) && entry.path().extension()==".lvl") {
            if (exists(entry.path()/"DoNotTest")) continue;
            std::cout << "Testing " << entry.path() << " ...";
            if (!exists(entry.path()/"win.replay")) {
                std::cerr << " Did not have win.replay\n";
                testPassed = false;
                continue;
            }
            auto start = std::chrono::high_resolution_clock::now();
            TimeEngine timeEngine = TimeEngine(loadLevelFromFile(entry.path()));
            auto const replay = loadReplay((entry.path()/"win.replay").string());
            try {
                for (auto const& input: replay) {
                    timeEngine.runToNextPlayerFrame(input);
                }
            }
            catch (PlayerVictoryException const&) {
                auto timeTaken =
                    std::chrono::duration_cast<std::chrono::duration<double>>(
                        std::chrono::high_resolution_clock::now()-start);
                std::cout << " OK, in: " << timeTaken.count() << "s\n";
                continue;
            }
            std::cerr << " Did not win\n";
            testPassed = false;
            continue;
        }
#endif
    }
    return testPassed;
}

struct tester {
    tester() {
        ::hg::getTestDriver().registerUnitTest(testLevels);
    }
} tester;
}
}
}
