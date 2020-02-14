#include "ReplayIO.h"
#include "hg/Util/TestDriver.h"
#include <sstream>
namespace hg {
namespace replay_io_test {
namespace {
    bool testReplaySavingAndLoading()
    {
        std::vector<InputList> inputReplay;
        inputReplay.push_back(InputList(GuyInput(false, true, false, false, false, false, false, false, Ability::NO_ABILITY, FrameID(), 0, 0), 0));
        inputReplay.push_back(InputList(GuyInput(false, true, false, false, false, false, false, false, Ability::NO_ABILITY, FrameID(), 0, 0), 0));
        inputReplay.push_back(InputList(GuyInput(false, false, false, false, false, false, false, true, Ability::TIME_JUMP, FrameID(35, UniverseID(20000)), 0, 0), -1));
        std::stringstream stream;
        saveReplay(stream, inputReplay);
        return inputReplay == loadReplay(stream);
    }
    struct tester final {
        tester() {
            ::hg::getTestDriver().registerUnitTest("ReplayIO_testReplaySavingAndLoading", testReplaySavingAndLoading);
        }
    } tester;
}
}
}
