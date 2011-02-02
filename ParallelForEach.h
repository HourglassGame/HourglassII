#ifndef HG_PARALLEL_FOR_EACH_H
#define HG_PARALLEL_FOR_EACH_H
#include <boost/range.hpp>
#include <tbb/parallel_do.h>
#include <cassert>
namespace hg {
//Const and non-const version to allow both const ranges and modifying Functions
template<typename SinglePassRange, typename Func>
void parallel_for_each(SinglePassRange& range, const Func& func)
{
    tbb::parallel_do(boost::begin(range), boost::end(range), func);
}
template<typename SinglePassRange, typename Func>
void parallel_for_each(const SinglePassRange& range, const Func& func)
{
    tbb::parallel_do(boost::begin(range), boost::end(range), func);
}
}
#endif //HG_PARALLEL_FOR_EACH_H
