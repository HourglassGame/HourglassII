#include "move_function.h"
#include "TestDriver.h"
namespace hg {
namespace move_function_test {
namespace {
    bool testOperatorBool()
    {
        bool passed(true);
        move_function<void()> a;
        passed = passed && !a;
        if (a) {
            passed = false;
        }
        a = &testOperatorBool;
        passed = passed && a;
        if (a) {
            //No problem
        }
        else {
            passed = false;
        }
        return passed;
    }

    struct tester {
        tester() {
            ::hg::getTestDriver().registerUnitTest("move_function_testOperatorBool", testOperatorBool);
        }
    } tester;
}
}
}

