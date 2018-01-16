#include "TestDriver.h"
namespace hg {
    namespace lua_test {
        namespace {
            //TODO: Test that throwing exceptions through lua stack frames works correctly and calls the appropriate destructors.
            //TODO: Test that throwing Lua Errors through C++ stack frames works correctly and calls the appropriate destructors

            //(Not sure which of the above two are meant to apply...)

            bool testThrowErrorThroughCxx()
            {
                
                return true;
            }
            struct tester final {
                tester() {
                    ::hg::getTestDriver().registerUnitTest("lua_testThrowErrorThroughCxx", testThrowErrorThroughCxx);
                }
            } tester;
        }
    }
}
