#include "Maths.h"
#include "TestDriver.h"
namespace hg {
namespace Maths_test {
namespace {
    bool testFlooredDivision() {
        return flooredDivision(-5,  3) == -2
            && flooredDivision(-4,  3) == -2
            && flooredDivision(-3,  3) == -1
            && flooredDivision(-2,  3) == -1
            && flooredDivision(-1,  3) == -1
            && flooredDivision( 0,  3) == 0
            && flooredDivision( 1,  3) == 0
            && flooredDivision( 2,  3) == 0
            && flooredDivision( 3,  3) == 1
            && flooredDivision( 4,  3) == 1
            
            && flooredDivision(-5, -3) == 1
            && flooredDivision(-4, -3) == 1
            && flooredDivision(-3, -3) == 1
            && flooredDivision(-2, -3) == 0
            && flooredDivision(-1, -3) == 0
            && flooredDivision( 0, -3) == 0
            && flooredDivision( 1, -3) == -1
            && flooredDivision( 2, -3) == -1
            && flooredDivision( 3, -3) == -1
            && flooredDivision( 4, -3) == -2
            && flooredDivision( 5, -3) == -2
            && flooredDivision( 6, -3) == -2;
    }


    bool testFlooredModulo()
    {
        return flooredModulo(-3,  3) == 0
            && flooredModulo(-2,  3) == 1
            && flooredModulo(-1,  3) == 2
            && flooredModulo( 0,  3) == 0
            && flooredModulo( 1,  3) == 1
            && flooredModulo( 2,  3) == 2
            && flooredModulo( 3,  3) == 0

            && flooredModulo(-3, -3) ==  0
            && flooredModulo(-2, -3) == -2
            && flooredModulo(-1, -3) == -1
            && flooredModulo( 0, -3) ==  0
            && flooredModulo( 1, -3) == -2
            && flooredModulo( 2, -3) == -1
            && flooredModulo( 3, -3) ==  0;
    }
    struct tester final {
        tester() {
            ::hg::getTestDriver().registerUnitTest("Maths_testFlooredDivision", testFlooredDivision);
            ::hg::getTestDriver().registerUnitTest("Maths_testFlooredModulo", testFlooredModulo);
        }
    } tester;
}
}
}
