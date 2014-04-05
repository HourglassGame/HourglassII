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
        return ptr.get()->a == 10;
    }
    bool testSwap()
    {
        clone_ptr<Cloneable> three(new Cloneable(3));
        clone_ptr<Cloneable> fourteen(new Cloneable(14));
        three.swap(fourteen);
        return three.get()->a == 14 && fourteen.get()->a == 3;
    }
    struct tester {
        tester() {
            ::hg::getTestDriver().registerUnitTest(testAssignment);
            ::hg::getTestDriver().registerUnitTest(testSwap);
        }
    } tester;
}
}
}
