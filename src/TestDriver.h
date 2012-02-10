#ifndef HG_TEST_DRIVER_H
#define HG_TEST_DRIVER_H
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <vector>
namespace hg {
//Singleton unit test driver.
//This is a stand-in until we have a build system
//that is capable of reliably producing and running unit-tests.
//This should at least allow us to start getting some benefits from testing.
//The idea is that unit tests get put into objects that are globally constructed,
//which register the tests in their constructors, and the first thing
//that main does is to run all the tests in the test driver.
class TestDriver : boost::noncopyable {
public:
    void registerUnitTest(boost::function<bool()> test);
    bool passesAllTests();
private:
    std::vector<boost::function<bool()> > tests;
};

TestDriver& getTestDriver();

}
#endif //HG_TEST_DRIVER_H
