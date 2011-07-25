#include "FrameUpdateSet.h"
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm/unique.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <boost/swap.hpp>
#include <cassert>
namespace hg {

FrameUpdateSet::FrameUpdateSet() :
#ifndef NDEBUG
        isSet_(true),
#endif
        updateSet_()
{
}
FrameUpdateSet::FrameUpdateSet(FrameUpdateSet const& other) :
#ifndef NDEBUG
        isSet_(other.isSet_),
#endif
        updateSet_(other.updateSet_)
{
}
FrameUpdateSet& FrameUpdateSet::operator=(FrameUpdateSet const& other)
{
#ifndef NDEBUG
    isSet_ = other.isSet_;
#endif
    updateSet_ = other.updateSet_;
    return *this;
}
void FrameUpdateSet::add(Frame* frame)
{
    assert(frame);
    updateSet_.push_back(frame);
#ifndef NDEBUG
    isSet_ = false;
#endif
}
void FrameUpdateSet::add(FrameUpdateSet const& other)
{
    updateSet_.insert(
        updateSet_.end(),
        other.updateSet_.begin(),
        other.updateSet_.end());
#ifndef NDEBUG
    isSet_ = false;
#endif
}
void FrameUpdateSet::swap(FrameUpdateSet& other)
{
    boost::swap(updateSet_, other.updateSet_);
#ifndef NDEBUG
    boost::swap(isSet_, other.isSet_);
#endif
}

void FrameUpdateSet::make_set()
{
    boost::erase(updateSet_, boost::unique<boost::return_found_end>(boost::sort(updateSet_)));
#ifndef NDEBUG
    isSet_ = true;
#endif
}

FrameUpdateSet::const_iterator
    FrameUpdateSet::begin() const {
    assert(isSet_);
    return updateSet_.begin();
}
FrameUpdateSet::const_iterator
    FrameUpdateSet::end() const {
    assert(isSet_);
    return updateSet_.end();
}
FrameUpdateSet::iterator
    FrameUpdateSet::begin() {
    assert(isSet_);
    return updateSet_.begin();
}
FrameUpdateSet::iterator
    FrameUpdateSet::end() {
    assert(isSet_);
    return updateSet_.end();
}
std::size_t FrameUpdateSet::size() const {
    assert(isSet_);
    return updateSet_.size();
}
bool FrameUpdateSet::empty() const {
    assert(isSet_);
    return updateSet_.empty();
}
bool FrameUpdateSet::operator==(
    FrameUpdateSet const& other)
{
    assert(isSet_);
    assert(other.isSet_);
    return updateSet_ == other.updateSet_;
}
}
