#ifndef HG_PARALLEL_FOR_EACH_H
#define HG_PARALLEL_FOR_EACH_H
#include <boost/range.hpp>
#include <tbb/parallel_do.h>
#include "TBBExceptionWrapper.h"
#include <boost/range/algorithm/for_each.hpp>
namespace hg {
namespace detail {
    //Const and non-const version to allow both const ranges and modifying Functions
    template<typename RandomAccessRange, typename Func>
    void parallel_for_each(
        RandomAccessRange& range, Func func, tbb::task_group_context& context)
    {
        tbb::parallel_do(boost::begin(range), boost::end(range), func, context);
        //boost::for_each(range, func);
    }
    template<typename RandomAccessRange, typename Func>
    void parallel_for_each(
        RandomAccessRange const& range, Func func, tbb::task_group_context& context)
    {
        tbb::parallel_do(boost::begin(range), boost::end(range), func, context);
        //boost::for_each(range, func);
    }
    template<typename RandomAccessRange, typename Func>
    struct parallel_for_each_struct {
        parallel_for_each_struct(RandomAccessRange& range, Func f, tbb::task_group_context& context) :
            range_(&range), f_(f), context_(&context) {}
        void operator()() const
        {
            ::hg::detail::parallel_for_each(*range_, f_, *context_);
        }
        RandomAccessRange *range_;
        Func f_;
        tbb::task_group_context *context_;
    };
}
//Const and non-const version to allow both const ranges and modifying Functions
template<typename RandomAccessRange, typename Func>
void parallel_for_each(
	RandomAccessRange& range, Func func, tbb::task_group_context& context)
{
    typedef TBBInnerExceptionWrapper<
        Func,
        void,
        typename boost::range_reference<RandomAccessRange>::type
    > InnerExceptionWrapper;
    typedef detail::parallel_for_each_struct<
        RandomAccessRange,
        InnerExceptionWrapper
    > InnerWrapper;
    TBBOuterExceptionWrapper<InnerWrapper>(InnerWrapper(range, InnerExceptionWrapper(func), context))();
}
template<typename RandomAccessRange, typename Func>
void parallel_for_each(
	RandomAccessRange const& range, Func func, tbb::task_group_context& context)
{
    typedef TBBInnerExceptionWrapper<
        Func,
        void,
        typename boost::range_reference<RandomAccessRange const>::type
    > InnerExceptionWrapper;
    typedef detail::parallel_for_each_struct<
        RandomAccessRange,
        InnerExceptionWrapper
    > InnerWrapper;
    TBBOuterExceptionWrapper<InnerWrapper>(InnerWrapper(range, InnerExceptionWrapper(func), context))();
}

template<typename RandomAccessRange, typename Func>
void parallel_for_each(
	RandomAccessRange& range, Func func)
{
    tbb::task_group_context context;
    parallel_for_each(range, func, context);
}
template<typename RandomAccessRange, typename Func>
void parallel_for_each(
	RandomAccessRange const& range, Func func)
{
    tbb::task_group_context context;
    parallel_for_each(range, func, context);
}
}
#endif //HG_PARALLEL_FOR_EACH_H
