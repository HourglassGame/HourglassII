#ifndef HG_PARALLEL_FOR_EACH_H
#define HG_PARALLEL_FOR_EACH_H
#include <boost/range.hpp>
#include <tbb/parallel_do.h>
#include <boost/range/algorithm/for_each.hpp>
#include "as_lvalue.h"

namespace hg {
template<typename RandomAccessRange, typename Func>
void parallel_for_each(
	RandomAccessRange &range, Func &&func, tbb::task_group_context &context = as_lvalue(tbb::task_group_context()))
{
    tbb::parallel_do(boost::begin(range), boost::end(range), std::forward<Func>(func), context);
    //boost::for_each(range, func);
}
}
#endif //HG_PARALLEL_FOR_EACH_H
