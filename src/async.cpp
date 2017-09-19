#include "async.h"
//#include <Windows.h>
//#include <ProcessThreadsApi.h>
#include <boost/thread/future.hpp>
#include <boost/container/stable_vector.hpp>
#include "move_function.h"
#include <functional>
namespace hg {
    class thread_comm_data {
    public:
        thread_comm_data(thread_pool *pool, move_function<void()> &&f) :
            m(),
            cv(),
            f(std::move(f)),
            pool(pool),
            ended(false) {}

        void thread_comm_data::become_idle() {
            pool->push_idle_thread(this);
        }
        move_function<void()> get_next() {
            std::unique_lock<std::mutex> l(m);
            if (!(f || ended)) {
                become_idle();
            }
            cv.wait(l, [&] {return f || ended; });
            move_function<void()> retF;
            boost::swap(retF, f);
            return { retF };
        }
        void set_f(move_function<void()> &&new_f) {
            {
                std::lock_guard<std::mutex> lock(m);
                assert(!f);
                assert(!ended);
                assert(new_f);
                f = std::move(new_f);
            }
            cv.notify_one();
        }
        void end() {
            {
                std::lock_guard<std::mutex> lock(m);
                assert(!ended);
                ended = true;
            }
            cv.notify_one();
        }
    private:
        std::mutex m;
        std::condition_variable cv;
        move_function<void()> f;
        thread_pool *pool;
        bool ended;
    };

    class pool_elem {
    public:
        pool_elem(thread_pool *pool, move_function<void()> &&f) :
            chan(pool, std::move(f)),
            thread([this] {
                while (auto const f = chan.get_next()) {
                    f();
                }
            })
        {
        
        }
        pool_elem(pool_elem *&) = delete;
        pool_elem& operator=(pool_elem &&) = delete;
        pool_elem(pool_elem const&) = delete;
        pool_elem& operator=(pool_elem const&) = delete;
        thread_comm_data chan;
        std::thread thread;
    };

    void thread_pool::pop_or_new(move_function<void()> &&f) {
        assert(f);
        //TODO: Add separate lock for pool and idle_threads?
        std::lock_guard<std::mutex> lock(m);
        if (!idle_threads.empty()) {
            thread_comm_data *chan = idle_threads.back();
            idle_threads.pop_back();
            chan->set_f(std::move(f));
        }
        else {
            pool.emplace_back(this, std::move(f));
            //SetThreadDescription(pool.back().thread.native_handle(), L"hg::async pool thread");
        }
    }
    void thread_pool::push_idle_thread(thread_comm_data *thread) {
        std::lock_guard<std::mutex> lock(m);
        idle_threads.push_back(thread);
    }
    thread_pool::~thread_pool() noexcept {
        for (auto &&e : pool) {
            e.chan.end();
        }
        //TODO: Join all threads simultaneously, if possible?
        for (auto &&e : pool) {
            e.thread.join();
        }
    }

    thread_pool &getSharedThreadPool() {
        static thread_pool thread_pool;
        return thread_pool;
    }
}
