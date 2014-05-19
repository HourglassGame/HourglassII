#include "ParallelForEach.h"
#include "TestDriver.h"
#include <vector>
namespace hg {
namespace parallel_for_each_test {
namespace {
    bool testMutatingFunctor()
    {
        std::vector<int> values{0,1,2,3,4};
        
        parallel_for_each(values, [](int &a) {a = a*2;});
        
        return values == std::vector<int>{0,2,4,6,8};
    }
    
    bool testConstRange() {
        bool worked = false;
        std::vector<int> const values{0,1,2,3,4};
        
        parallel_for_each(values, [&](int a) {if (a == 3) worked = true;});
        
        return worked;
    
    }

    struct tester {
        tester() {
            ::hg::getTestDriver().registerUnitTest(testMutatingFunctor);
            ::hg::getTestDriver().registerUnitTest(testConstRange);
        }
    } tester;
}
}
}

