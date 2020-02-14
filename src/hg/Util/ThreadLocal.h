#ifndef HG_THREAD_LOCAL_H
#define HG_THREAD_LOCAL_H
#include <map>
#include <tbb/task_scheduler_observer.h>
#include <thread>
#include <tbb/spin_rw_mutex.h>
namespace hg {
template<typename T>
class ThreadLocal final : private tbb::task_scheduler_observer
{
    //For practical purposes the mutex should almost never be contended,
    //so a (lightweight) spin_mutex is preferable.
    //This has not actually been tested though.
    typedef tbb::spin_rw_mutex Mutex;
    typedef std::map<std::thread::id, T> Map;
public:
    ThreadLocal() :
        threadLocalData(),
        mut()
    {}
    T &get() {
        {
            Mutex::scoped_lock l(mut, false);
            typename Map::iterator it(
                threadLocalData.find(std::this_thread::get_id()));
            if (it != threadLocalData.end()) {
                return it->second;
            }
        }
        {
            Mutex::scoped_lock l(mut, true);
            return threadLocalData
                .emplace(
                    std::piecewise_construct,
                    std::make_tuple(std::this_thread::get_id()),
                    std::make_tuple())
                .first->second;
            //return threadLocalData[std::this_thread::get_id()];
        }
    }
    ThreadLocal(ThreadLocal const &o) : threadLocalData(), mut() {
        Mutex::scoped_lock l(o.mut, false);
        threadLocalData = o.threadLocalData;
    }

    ThreadLocal &operator=(ThreadLocal const &o) {
        if (&o != this) {
            Mutex::scoped_lock l1(mut, true);
            Mutex::scoped_lock l2(o.mut, false);
            threadLocalData = o.threadLocalData;
        }
        return *this;
    }
private:
    
    virtual void on_scheduler_exit(bool /*is_worker*/) override {
        //Note that this clears the element even for non-worker threads.
        //This means that the cache may sometimes be spuriously be cleared if
        //this class is used with non-tbb-task threads.
        Mutex::scoped_lock l(mut, false);
        typename Map::iterator it(
            threadLocalData.find(std::this_thread::get_id()));
        if (it == threadLocalData.end()) return;
        l.release();
        //Relying on the fact that `it` cannot be invalidated
        //to get through this unlocked region.
        l.acquire(mut, true);
        threadLocalData.erase(it);
    }

    Map threadLocalData;
    mutable Mutex mut;
};
} //namespace hg
#endif //HG_THREAD_LOCAL_H
