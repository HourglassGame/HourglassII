#include "FrameUpdateSet.h"
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm/unique.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <boost/swap.hpp>
#include <cassert>
namespace hg {

FrameUpdateSet::FrameUpdateSet() :
        isSet_(true),
        updateSet_()
{
}
FrameUpdateSet::FrameUpdateSet(FrameUpdateSet const& other) :
        isSet_(other.isSet_),
        updateSet_(other.updateSet_)
{
}
FrameUpdateSet& FrameUpdateSet::operator=(FrameUpdateSet const& other)
{
    isSet_ = other.isSet_;
    updateSet_ = other.updateSet_;
    return *this;
}
void FrameUpdateSet::add(Frame* frame)
{
    assert(frame);
    updateSet_.push_back(frame);
    isSet_ = false;
}
void FrameUpdateSet::add(FrameUpdateSet const& other)
{
    updateSet_.insert(
        updateSet_.end(),
        other.updateSet_.begin(),
        other.updateSet_.end());
    isSet_ = false;
}
void FrameUpdateSet::swap(FrameUpdateSet& other)
{
    boost::swap(updateSet_, other.updateSet_);
    boost::swap(isSet_, other.isSet_);
}

void FrameUpdateSet::make_set() const
{
    if (!isSet_) {
        boost::erase(updateSet_, boost::unique<boost::return_found_end>(boost::sort(updateSet_)));
        isSet_ = true;
    }
}

FrameUpdateSet::const_iterator
    FrameUpdateSet::begin() const {
    make_set();
    return updateSet_.begin();
}
FrameUpdateSet::const_iterator
    FrameUpdateSet::end() const {
    make_set();
    return updateSet_.end();
}
FrameUpdateSet::iterator
    FrameUpdateSet::begin() {
    make_set();
    return updateSet_.begin();
}
FrameUpdateSet::iterator
    FrameUpdateSet::end() {
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
    FrameUpdateSet const& other)
{
    make_set();
    other.make_set();
    return updateSet_ == other.updateSet_;
}
}
