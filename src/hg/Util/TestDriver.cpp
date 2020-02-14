#include "TestDriver.h"
#include <boost/range/algorithm/for_each.hpp>
#include <iostream>
#include <string>
namespace hg {
void TestDriver::registerUnitTest(std::string testName, std::function<bool()> test)
{
    tests.emplace_back(std::move(testName), std::move(test));
}

bool TestDriver::passesAllTests() {
    bool allTestsPassed = true;
    for (auto const& test: tests) {
        std::cerr << "Testing: " << std::get<0>(test) << "\n" << std::flush;
        if (!std::get<1>(test)()) {
            allTestsPassed = false;
            std::cerr << "Failed: " << std::get<0>(test) << "\n";
        }
    }
    return allTestsPassed;
}

TestDriver &getTestDriver() {
    static TestDriver instance;
    return instance;
}
#if 0
bool testExpression = true;

struct TestDriverTest final : Test {
    virtual bool run(Outputter &o) override {
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
