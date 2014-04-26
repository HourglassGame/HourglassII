#include "natural_sort.h"
#include "TestDriver.h"
namespace hg {
namespace natural_sort_test {
namespace {
    bool test_natural_less()
    {
        return natural_less("6StackedHigh","7Timebelt") == true
            && natural_less("7Timebelt", "6StackedHigh") == false
            && natural_less("6StackedHigh", "6StackedHigh") == false
            && natural_less("6StackedHigh", "6stackedhigh") == false
            && natural_less("6stackedhigh", "6StackedHigh") == false
            && natural_less("6stackedhigh10", "6StackedHigh10") == false
            && natural_less("6stackedhigh9", "6StackedHigh10") == true
            && natural_less("6StackedHigh9", "6stackedhigh10") == true
            ;
    }
    struct tester {
        tester() {
            ::hg::getTestDriver().registerUnitTest(test_natural_less);
        }
    } tester;
}
}
}
