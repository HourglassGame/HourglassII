#ifndef HG_ENQUEUE_TASK_H
#define HG_ENQUEUE_TASK_H
#include <boost/thread/future.hpp>
#include <tbb/task_group.h>

template<typename F>
boost::future<typename boost::result_of<F()>::type> enqueue_task(tbb::task_group& queue, F f)
{
    auto task = std::make_shared<boost::packaged_task<typename boost::result_of<F()>::type()>>(f);
    auto copyable_task = [=] {return (*task)();};
    boost::future<typename boost::result_of<F()>::type> future(task->get_future());
    queue.run(copyable_task);
    return boost::move(future);
}
#endif //HG_ENQUEUE_TASK_H
