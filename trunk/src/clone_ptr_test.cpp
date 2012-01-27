#include "clone_ptr.h"
#include "TestDriver.h"
namespace hg {
namespace clone_ptr_test {
namespace {
    struct Cloneable {
        Cloneable(int na) : a(na) {}
        int a;
    };
    bool testAssignment()
    {
        clone_ptr<Cloneable> ptr(new Cloneable(5));
        ptr = clone_ptr<Cloneable>(new Cloneable(10));
        return ptr.get().a == 10;
    }
    struct tester {
        tester() {
            ::hg::getTestDriver().registerUnitTest(testAssignment);
        }
    } tester;
}
}
}
