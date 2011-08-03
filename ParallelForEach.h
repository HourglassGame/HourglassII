#ifndef HG_PARALLEL_FOR_EACH_H
#define HG_PARALLEL_FOR_EACH_H
#include <boost/range.hpp>
#include <tbb/parallel_do.h>
//#include <boost/range/algorithm/for_each.hpp>
namespace hg {
//Const and non-const version to allow both const ranges and modifying Functions
template<typename RandomAccessRange, typename Func>
void parallel_for_each(RandomAccessRange& range, Func func)
{
    tbb::parallel_do(boost::begin(range), boost::end(range), func);
    //boost::for_each(range, func);
}
template<typename RandomAccessRange, typename Func>
void parallel_for_each(RandomAccessRange const& range, Func func)
{
    tbb::parallel_do(boost::begin(range), boost::end(range), func);
    //boost::for_each(range, func);
}
}
#endif //HG_PARALLEL_FOR_EACH_H
