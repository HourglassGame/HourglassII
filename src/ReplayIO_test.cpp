#include "ReplayIO.h"
#include "TestDriver.h"
#include <sstream>
namespace hg {
namespace replay_io_test {
namespace {
    bool testReplaySavingAndLoading()
    {
        std::vector<InputList> inputReplay;
        inputReplay.push_back(InputList(false, true, false, false, false, false, NO_ABILITY, FrameID(), 0, 0));
        inputReplay.push_back(InputList(false, true, false, false, false, false,NO_ABILITY, FrameID(), 0, 0));
        inputReplay.push_back(InputList(false, false, false, false, false, true, TIME_JUMP, FrameID(35, UniverseID(20000)), 0, 0));
        std::stringstream stream;
        saveReplay(stream, inputReplay);
        return inputReplay == loadReplay(stream);
    }
    struct tester {
        tester() {
            ::hg::getTestDriver().registerUnitTest(testReplaySavingAndLoading);
        }
    } tester;
}
}
}
