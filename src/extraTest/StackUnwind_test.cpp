#include "hg/Util/TestDriver.h"
#include <iostream>

namespace hg {
namespace StackUnwind_test {
namespace {
    static bool doubleDestructed = false;
    struct TestException {};
    struct TestDtor
    {
        TestDtor() : objectConstructed(0xFAFAFAFA)
        {}
        ~TestDtor() {
            if (objectConstructed != 0xFAFAFAFA) {
                doubleDestructed = true;
                std::cout << "";
            }
            objectConstructed = 0;
        }
    private:
        unsigned objectConstructed;
    };
    struct FlaggedNonTrivialDtor
    {
    private:
        bool hasValue;
    public:
        FlaggedNonTrivialDtor(bool hasValue = false) : hasValue(hasValue)
        {}
        operator bool() {
            return hasValue;
        }
        ~FlaggedNonTrivialDtor() {
            if (hasValue) {
                std::cout << "";
            }
        }
    };
    struct NonTrivialDtor
    {
        ~NonTrivialDtor() {
            std::cout << "";
        }
    };
    TestDtor get(FlaggedNonTrivialDtor &flag)
    {
        //Removing the 'a' variable eliminates the problem.
        NonTrivialDtor a;
        if (flag) {
            throw TestException{};
        }
        //The problem is here!
        //TestDtor gets destructed twice!
        return TestDtor{};
    }
    //Tests for presence of bug that appeared in MSVC in visual studio 15.3.5
    //https://developercommunity.visualstudio.com/content/problem/118080/c-incorrect-code-generation-destructor-being-calle.html
    //This problem affected the handling of PlayerVictoryException, as it caused double-destruction of the RunResult object,
    //meaning that winning a level would crash the game.
    bool testStackUnwind()
    {
        doubleDestructed = false;
        int counter = 0;
        while (true) {
            {
                ++counter;
                FlaggedNonTrivialDtor flag;
                //Problem only occurs if this lexical section of code has been run at least once before.
                //So at a minimum, for the problem to occur;
                //x must be at least 2 in 'counter >= x'.
                //If you manually unroll the loop, the problem goes away unless you also increase the 'x'.
                if (counter >= 2) {
                    flag = FlaggedNonTrivialDtor(true);
                }
                try {
                    get(flag);
                }
                catch (TestException const &) {
                    return !doubleDestructed;
                }
            }
        }
        return false;
    }

    struct tester final {
        tester() {
            ::hg::getTestDriver().registerUnitTest("StackUnwind_testStackUnwind", testStackUnwind);
        }
    } tester;
}
}
}
