#include "FrameUpdateSet.h"
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm/unique.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <boost/swap.hpp>
#include <tbb/parallel_sort.h>
#include <utility>
#include <cassert>

namespace hg {

FrameUpdateSet::FrameUpdateSet() :
        isSet_(true),
        updateSet_()
{
}
void FrameUpdateSet::add(Frame *frame)
{
    assert(frame);
    updateSet_.push_back(frame);
    isSet_ = false;
}
void FrameUpdateSet::add(FrameUpdateSet const &o)
{
    updateSet_.insert(
        updateSet_.end(),
        o.updateSet_.begin(),
        o.updateSet_.end());
    isSet_ = false;
}
void FrameUpdateSet::swap(FrameUpdateSet &o) noexcept
{
    boost::swap(updateSet_, o.updateSet_);
    boost::swap(isSet_, o.isSet_);
}

void FrameUpdateSet::make_set() const
{
    if (!isSet_) {
        tbb::parallel_sort(updateSet_.begin(), updateSet_.end());
        boost::erase(updateSet_, boost::unique<boost::return_found_end>(updateSet_));
        isSet_ = true;
    }
}

FrameUpdateSet::const_iterator
    FrameUpdateSet::begin() const
{
    make_set();
    return updateSet_.begin();
}
FrameUpdateSet::const_iterator
    FrameUpdateSet::end() const
{
    make_set();
    return updateSet_.end();
}
FrameUpdateSet::iterator
    FrameUpdateSet::begin()
{
    make_set();
    return updateSet_.begin();
}
FrameUpdateSet::iterator
    FrameUpdateSet::end()
{
    make_set();
    return updateSet_.end();
}
std::size_t FrameUpdateSet::size() const {
    make_set();
    return updateSet_.size();
}
bool FrameUpdateSet::empty() const {
    return updateSet_.empty();
}
bool FrameUpdateSet::operator==(
    FrameUpdateSet const &o)
{
    make_set();
    o.make_set();
    return updateSet_ == o.updateSet_;
}
}
