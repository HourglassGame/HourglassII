#ifdef HG_COMPILE_TESTS
#include "TestDriver.h"
#include <boost/range/algorithm/for_each.hpp>
namespace hg {

void TestDriver::registerUnitTest(boost::function<bool()> test)
{
    tests.push_back(test);
}

struct Run {
    Run(bool& allTestsPassed) :
    allTestsPassed_(allTestsPassed)
    {
    }
    template<typename Callable>
    void operator()(Callable const& func) {
        if(!func()) {
            allTestsPassed_ = false;
        }
    }
    bool& allTestsPassed_;
};

bool TestDriver::passesAllTests() {
    bool allTestsPassed = true;
    boost::for_each(tests, Run(allTestsPassed));
    return allTestsPassed;
}

TestDriver& getTestDriver()
{
    static TestDriver instance;
    return instance;
}
}

#endif //HG_COMPILE_TESTS
