//NOT YET IMPLEMENTED!
//This file contains test cases intended to ensure that every level in the
//game is playable.

//The way it works currently is make sure that every `.lua` file in the levels folder,
//has a corresponding `.replay` file, and that the corresponding `.replay` file
//results in the level being won.
#include "TestDriver.h"
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
    //TODO
    return true;
}

struct tester {
    tester() {
        ::hg::getTestDriver().registerUnitTest(testLevels);
    }
} tester;
}
}
}
