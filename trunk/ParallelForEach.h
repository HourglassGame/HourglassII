#ifndef HG_PARALLEL_FOR_EACH_H
#define HG_PARALLEL_FOR_EACH_H
#include "FrameID.h"
//#include <boost/range/algorithm/for_each.hpp>
#include <tbb/parallel_for_each.h>
//#include <tbb/parallel_for.h>
#include <algorithm>
#include <vector>
#include <iostream>
namespace hg {
template<typename SinglePassRange, typename Func>
void parallel_for_each(SinglePassRange& range, const Func& func)
{
    //#if defined HG_TBB_SUPPORTED
    //tbb::parallel_for(range, func);
    //std::vector<FrameID> rangeCopy(range);
    tbb::parallel_for_each(range.begin(), range.end(), func);
    //assert(range==rangeCopy);
    //boost::begin(range), boost::end(range), func);
    //#else
    //parallel_for_each_impl(chunker<range::iterator>());
    //boost::for_each(range, func);
    //#endif
}
}
#endif //HG_PARALLEL_FOR_EACH_H
