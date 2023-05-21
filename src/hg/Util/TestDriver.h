#ifndef HG_TEST_DRIVER_H
#define HG_TEST_DRIVER_H
#include <functional>
#include <boost/noncopyable.hpp>
#include <vector>
#include <tuple>
#include <string>
namespace hg {
#if 0
struct TestMonitor {
//    void startingSuite
};

struct BugCounter {
};

struct signal_caught_tag{};
template<typename T>
variant<T, exception_ptr, signal_caught_tag> executeChecked(std::function<T()> f);



struct Test {
    virtual void run(BugCounter &bugCount) = 0;
};
#endif


//Singleton unit test driver.
//This is a stand-in until we have a build system
//that is capable of reliably producing and running unit-tests.
//This should at least allow us to start getting some benefits from testing.
//The idea is that unit tests get put into objects that are globally constructed,
//which register the tests in their constructors, and the first thing
//that main does is to run all the tests in the test driver.
class TestDriver final : boost::noncopyable {
public:
    void registerUnitTest(std::string testName, std::function<bool()> test);
    bool passesAllTests(/*TestMonitor &*/);
private:
    std::vector<std::tuple<std::string,std::function<bool()>>> tests;
};

TestDriver &getTestDriver();

}
#endif //HG_TEST_DRIVER_H
