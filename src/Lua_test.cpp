#include "TestDriver.h"
namespace hg {
    namespace lua_test {
        namespace {
            //TODO: Test that throwing exceptions through lua stack frames works correctly and calls the appropriate destructors/lua cleanup functions.
            //Examples requiring this in Hourglass:
            // panic function used to handle out-of-memory, interruption and unhandled lua runtime exceptions (SimpleLuaCpp.h)

            //TODO: Test that throwing Lua Errors through C++ stack frames works correctly and calls the appropriate destructors
            //Examples requiring this in Hourglass:
            // In theory, any lua_CFunction that calls into lua could encounter this.
            // Not sure if this ever actually happens in HGII.
            // Either make sure it works correctly, or be sure to only ever use pcall from
            // inside lua_CFunctions.



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
