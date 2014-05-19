#ifndef HG_PARALLEL_FOR_EACH_H
#define HG_PARALLEL_FOR_EACH_H
#include <boost/range.hpp>
#include <tbb/parallel_do.h>
#include <boost/range/algorithm/for_each.hpp>
namespace hg {
template<typename RandomAccessRange, typename Func>
void parallel_for_each(
	RandomAccessRange &range, Func func, tbb::task_group_context &context)
{
    tbb::parallel_do(boost::begin(range), boost::end(range), func, context);
    //boost::for_each(range, func);
}

template<typename RandomAccessRange, typename Func>
void parallel_for_each(
	RandomAccessRange &range, Func func)
{
    tbb::task_group_context context;
    parallel_for_each(range, func, context);
}
}
#endif //HG_PARALLEL_FOR_EACH_H
