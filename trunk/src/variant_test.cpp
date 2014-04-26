#include "variant.h"
#include "TestDriver.h"
namespace hg {
namespace variant_test {
namespace {

    struct test_Visitor {
        typedef void result_type;
        
        result_type operator()(int) {}
        result_type operator()(double) {
            worked = false;
        }
        
        bool &worked;
    };
    
    struct test_MultiVisitor {
        typedef void result_type;
        
        result_type operator()(int, int) {
            worked = false;
        }
        result_type operator()(int, double) {
        }
        result_type operator()(double, int) {
            worked = false;
        }
        result_type operator()(double, double) {
            worked = false;
        }
        
        bool &worked;
    };

    bool test_apply_visitor() {
        bool worked = true;
        variant<int, double> test_variantA(10);
        apply_visitor(test_Visitor{worked}, test_variantA);
        
        variant<int, double> test_variantB(5.);
        apply_visitor(test_MultiVisitor{worked}, test_variantA, test_variantB);
        
        return worked;
    }

    struct tester {
        tester() {
            ::hg::getTestDriver().registerUnitTest(test_apply_visitor);
        }
    } tester;
}
}
}
