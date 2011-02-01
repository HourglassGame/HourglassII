#ifndef HG_PARALLEL_FOR_EACH_H
#define HG_PARALLEL_FOR_EACH_H
#include <boost/range.hpp>
#include <tbb/parallel_do.h>
#include <cassert>
namespace hg {
template<typename SinglePassRange, typename Func>
void parallel_for_each(SinglePassRange& range, const Func& func)
{
    //parallel_do shouldn't modify, but won't compile when passed const Range.
    //Probably because of the alternative form where items can be added 
    //to the range by the processing of earlier items 
    #ifndef NDEBUG
    //SinglePassRange rangeCopy(range);
    #endif
    tbb::parallel_do(boost::begin(range), boost::end(range), func);
    #ifndef NDEBUG
    //assert(range==rangeCopy);
    #endif
}
}
#endif //HG_PARALLEL_FOR_EACH_H
