#ifndef HG_THREAD_LOCAL_H
#define HG_THREAD_LOCAL_H
#include "mt/std/map"
#include <tbb/task_scheduler_observer.h>
#include <tbb/compat/thread>
#include <tbb/spin_rw_mutex.h>
namespace hg {
template<typename T>
class ThreadLocal : private tbb::task_scheduler_observer
{
    //For practical purposes the mutex should almost never be contended,
    //so a (lightweight) spin_mutex is preferable.
    //This has not actually been tested though.
    typedef tbb::spin_rw_mutex Mutex;
    typedef typename mt::std::map<std::thread::id, T>::type Map;
public:
    ThreadLocal(){}
    T& get() {
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
            return threadLocalData[std::this_thread::get_id()];
        }
    }
private:
    //intentionally undefined
    ThreadLocal(ThreadLocal const&);
    ThreadLocal& operator=(ThreadLocal const&);
    
    virtual void on_scheduler_exit(bool /*is_worker*/) {
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
    Mutex mut;
};
} //namespace hg
#endif //HG_THREAD_LOCAL_H
