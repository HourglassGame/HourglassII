#include "TestDriver.h"
#include <boost/range/algorithm/for_each.hpp>
namespace hg {
void TestDriver::registerUnitTest(std::function<bool()> test)
{
    tests.push_back(test);
}

bool TestDriver::passesAllTests() {
    bool allTestsPassed = true;
    for (auto const& test: tests) {
        if (!test()) allTestsPassed = false;
    }
    return allTestsPassed;
}

TestDriver &getTestDriver() {
    static TestDriver instance;
    return instance;
}
#if 0
bool testExpression = true;

struct TestDriverTest: Test {
    virtual bool run(Outputter &o) {
        switch (runMonitored([&]{return testExpression;})) {
            case OK: break;
            case BAD_VALUE: reportBadValue(); break;
            case EXCEPTION_THROWN: reportException(); break;
            case SEGFAULT: reportSegfault(); break;
        }
    }
};
#endif

}
