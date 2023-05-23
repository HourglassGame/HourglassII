#include "async.h"
#include "TestDriver.h"
namespace hg {
namespace async_test {
namespace {
    bool testAsyncExecution()
    {
        //Checks that 'hg::async' actually runs the created task on a new thread that will
        //progress, even if all other threads are blocked.

        //TODO : Do hardcore test with super large number (>10000?) of concurrent
        //       enqueued items that must all progress.
//#if 0
        auto test = ([] {
            //Running test in newly created thread so task_scheduler_init is able to operate independently
            //from the main codebase.
            //tbb::task_scheduler_init init(1); //No longer exists. TODO: Is there another way to force TBB into single threaded mode?
                                                //Not that it is particularly relevant right now, as the current implementation of hg::async does not use TBB
            std::atomic<int> counter(0);
            async([&] {
                async([&] {
                    while (counter < 5) {}
                    ++counter;

                    while (counter < 9) {}
                    ++counter;
                });
                while (counter < 1) {}
                ++counter;
                while (counter < 3) {}
                ++counter;
                while (counter < 6) {}
                ++counter;

                while (counter < 10) {}
                ++counter;
            });
            async([&] {
                while (counter < 8) {}
                ++counter;
                while (counter < 12) {}
                ++counter;
            });
            ++counter;
            while (counter < 2) {}
            ++counter;
            while (counter < 4) {}
            ++counter;
            while (counter < 7) {}
            ++counter;
            while (counter < 11) {}
            ++counter;
            while (counter < 13) {}
        });

        boost::thread t(test);
        t.join();
//#endif

        //This test will never return if async does not ensure that all
        //enqueued tasks get to run in parallel.
        return true;
    }
    struct tester final {
        tester() {
            ::hg::getTestDriver().registerUnitTest("async_testAsyncExecution", testAsyncExecution);
        }
    } tester;
}
}
}
