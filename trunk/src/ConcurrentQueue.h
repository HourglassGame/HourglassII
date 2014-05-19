#ifndef HG_CONCURRENT_QUEUE_H
#define HG_CONCURRENT_QUEUE_H
#include "multi_thread_allocator.h"
#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>
#include <boost/container/deque.hpp>
#include <utility>

namespace hg {
//Single-reader single-writer concurrent queue.
//(Quite possibly can handle multi-reader multi-writer too, check before using)
template<typename T, typename Alloc = typename multi_thread_allocator_t<T>>
class ConcurrentQueue
{
public:
    //Not safe for concurrent execution
    ConcurrentQueue() = default;
    ConcurrentQueue(ConcurrentQueue const &o) :
        mutex(),
        cond(),
        queue(o.queue)
    {
    }
    ConcurrentQueue &operator=(ConcurrentQueue const &o)
    {
        queue = o.queue;
        return *this;
    }
    ConcurrentQueue(ConcurrentQueue &&o) :
        mutex(),
        cond(),
        queue(std::move(o.queue))
    {
    }
    ConcurrentQueue &operator=(ConcurrentQueue &&o)
    {
        queue = std::move(o.queue);
        return *this;
    }
    
    //push can be safely executed while pop is executing in a different thread.
    void push(T &&toPush)
    {
        bool shouldNotify;
        {
            boost::lock_guard<boost::mutex> lock(mutex);
            shouldNotify = queue.empty();
            queue.push_back(std::move(toPush));
        }
        if (shouldNotify) {
            cond.notify_one();
        }
    }
    //pop can be safely executed while push is executing in a different thread.
    //If the queue is empty available, pop() blocks (in a Boost "Interruption Point")
    //until an element is available.
    T pop()
    {
        boost::unique_lock<boost::mutex> lock(mutex);
        while (queue.empty()) {
            cond.wait(lock);
        }
        T retv(std::move(queue.front()));
        queue.pop_front();
        static_assert(noexcept(T{std::declval<T>()}),"Objects popped from ConcurrentQueue must have no-throw move ctors");
        return std::move(retv);
    }
private:
    mutable boost::mutex mutex;
    mutable boost::condition_variable cond;
    boost::container::deque<T, Alloc> queue;
};
}
#endif //HG_CONCURRENT_QUEUE_H
