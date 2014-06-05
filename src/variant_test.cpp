#include "variant.h"
#include "TestDriver.h"
namespace hg {
namespace variant_test {
namespace {
    //TODO: these tests are decidedly non-conclusive.
    //Add more tests that better capture the whole requirements of a variant
    struct A{};
    struct B{};

    struct test_Visitor {
        typedef void result_type;
        
        result_type operator()(A) {}
        result_type operator()(B) {
            worked = false;
        }
        
        bool &worked;
    };
    
    struct test_MultiVisitor {
        typedef void result_type;
        
        result_type operator()(A, A) {
            worked = false;
        }
        result_type operator()(A, B) {
        }
        result_type operator()(B, A) {
            worked = false;
        }
        result_type operator()(B, B) {
            worked = false;
        }
        
        bool &worked;
    };

    bool test_apply_visitor() {
        bool worked = true;

        variant<A, B> test_variantA(A{});
        apply_visitor(test_Visitor{worked}, test_variantA);
        
        variant<A, B> test_variantB(B{});
        apply_visitor(test_MultiVisitor{worked}, test_variantA, test_variantB);
        
        return worked;
    }
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-member-function"
#endif
    struct test_RefCategoryVisitor {
        typedef void result_type;
        bool &worked;
        
        result_type operator()(A&, A&) {
            worked = false;
        }
        result_type operator()(A&, B&) {
            worked = false;
        }
        result_type operator()(B&, A&) {
            worked = false;
        }
        result_type operator()(B&, B&) {
            worked = false;
        }
        
        
        result_type operator()(A&&, A&&) {
            worked = false;
        }
        result_type operator()(A&&, B&&) {
            worked = false;
        }
        result_type operator()(B&&, A&&) {
            worked = false;
        }
        result_type operator()(B&&, B&&) {
            worked = false;
        }
        
        
        result_type operator()(A&, A&&) {
            worked = false;
        }
        result_type operator()(A&, B&&) {
            worked = false;
        }
        result_type operator()(B&, A&&) {
            worked = false;
        }
        result_type operator()(B&, B&&) {
            worked = false;
        }
        
        result_type operator()(A&&, A&) {
            worked = false;
        }
        result_type operator()(A&&, B&) {
            //worked = false;
        }
        result_type operator()(B&&, A&) {
            worked = false;
        }
        result_type operator()(B&&, B&) {
            worked = false;
        }
    };

    struct test_RefCatVis0 {
        typedef void result_type;
        result_type operator()(A&&) {
        
        }
    };
    
    struct test_RefCatVis1 {
        typedef void result_type;
        result_type operator()(A&&) {
        }
        result_type operator()(B&&) {
        }
    };
    struct test_RefCatVis2 {
        typedef void result_type;
        bool &worked;
        result_type operator()(A&, A&) {
            worked = false;
        }
        result_type operator()(A&, A&&) {
        }
        result_type operator()(A&&, A&) {
            worked = false;
        }
        result_type operator()(A&&, A&&) {
            worked = false;
        }
    };
#ifdef __clang__
#pragma clang diagnostic pop
#endif
    bool test_apply_visitor_forwarding() {
        bool worked = true;
        variant<A, B> test_variantB(B{});
        apply_visitor(test_RefCategoryVisitor{worked}, variant<A, B>(A{}), test_variantB);
        
        variant<A>(A{}).visit(test_RefCatVis0{});
        
        test_RefCatVis1{}(A{});
        
        variant<A,B>{A{}}.visit(test_RefCatVis1{});
        
        variant<A> test_variant{A{}};
        apply_visitor(test_RefCatVis2{worked}, test_variant, variant<A>(A{}));
        return worked;
    }

    struct tester {
        tester() {
            ::hg::getTestDriver().registerUnitTest(test_apply_visitor);
            ::hg::getTestDriver().registerUnitTest(test_apply_visitor_forwarding);
        }
    } tester;
}
}
}
