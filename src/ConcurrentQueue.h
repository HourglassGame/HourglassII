#ifndef HG_CONCURRENT_QUEUE_H
#define HG_CONCURRENT_QUEUE_H
#include "multi_thread_allocator.h"
#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>
#include <boost/container/deque.hpp>
#include <boost/move/move.hpp>
namespace hg {
//Single-reader single-writer concurrent queue.
//(Quite possible can handle multi-reader multi-writer too, check before using)
template<typename T, typename Alloc = typename multi_thread_allocator<T>::type >
class ConcurrentQueue
{
public:
    //Not safe for concurrent execution
    ConcurrentQueue() :
        mutex_(),
        cond_(),
        queue_()
    {}
    ConcurrentQueue(ConcurrentQueue const& o) :
        mutex_(),
        cond_(),
        queue_(o.queue_)
    {
    }
    ConcurrentQueue& operator=(BOOST_COPY_ASSIGN_REF(ConcurrentQueue) o)
    {
        queue_ = o.queue_;
        return *this;
    }
    ConcurrentQueue(BOOST_RV_REF(ConcurrentQueue) o) :
        mutex_(),
        cond_(),
        queue_(boost::move(o.queue_))
    {
    }
    ConcurrentQueue& operator=(BOOST_RV_REF(ConcurrentQueue) o)
    {
        queue_ = boost::move(o.queue_);
        return *this;
    }
    
    //push can be safely executed while pop is executing in a different thread.
    void push(BOOST_RV_REF(T) toPush)
    {
        bool shouldNotify;
        {
            boost::lock_guard<boost::mutex> lock(mutex_);
            shouldNotify = queue_.empty();
            queue_.push_back(boost::move(toPush));
        }
        if (shouldNotify) {
            cond_.notify_one();
        }
    }
    //pop can be safely executed while push is executing in a different thread.
    //If the queue is empty available, pop() blocks (in a Boost "Interruption Point")
    //until an element is available.
    T pop()
    {
        boost::unique_lock<boost::mutex> lock(mutex_);
        while (queue_.empty()) {
            cond_.wait(lock);
        }
        T retv(boost::move(queue_.front()));
        queue_.pop_front();
        return boost::move(retv);
    }
private:
    boost::mutex mutex_;
    boost::condition_variable cond_;
    boost::container::deque<T, Alloc> queue_;
    BOOST_COPYABLE_AND_MOVABLE(ConcurrentQueue)
};
}
#endif //HG_CONCURRENT_QUEUE_H
