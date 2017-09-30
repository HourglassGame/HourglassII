#ifndef HG_ASYNC_H
#define HG_ASYNC_H
#include "move_function.h"
#include <boost/container/stable_vector.hpp>
#include <boost/thread/future.hpp>
#include <future>
namespace hg {
class pool_elem;
class thread_comm_data;
class thread_pool final {
private:
    mutable std::mutex m;
    boost::container::stable_vector<pool_elem> pool;
    boost::container::stable_vector<thread_comm_data*> idle_threads;
public:
    void pop_or_new(move_function<void()> &&f);
    void push_idle_thread(thread_comm_data *thread);
    ~thread_pool() noexcept;
};
thread_pool &getSharedThreadPool();

//hg::async(f)
//Approximatly equivalent to std::async(std::launch::async, f),
//except that it uses a pool of threads (for improved performance).
//That is, hg::async is guaranteed to run f on a separate thread,
//but it caches and reuses threads, so 'f' might not be run on a new thread,
//as it would be in std::asyng.
template<typename F>
auto async(F &&f) -> boost::future<decltype(f())>
{
    thread_pool &thread_pool = getSharedThreadPool();

    auto task{ boost::packaged_task<decltype(f())()>(std::forward<F>(f)) };
    auto future{ task.get_future() };

    thread_pool.pop_or_new(std::move(task));
    return future;
}
}
#endif //HG_ASYNC_H
