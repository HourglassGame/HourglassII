#include "FrameUpdateSet.h"
#include <boost/range/algorithm/equal.hpp>
#include <cassert>
namespace hg {
FrameUpdateSet::FrameUpdateSet() :
        updateSet_()
{
}

FrameUpdateSet::FrameUpdateSet(FrameUpdateSet const& other) :
        updateSet_(other.updateSet_)
{
}
FrameUpdateSet& FrameUpdateSet::operator=(BOOST_COPY_ASSIGN_REF(FrameUpdateSet) other)
{
    updateSet_ = other.updateSet_;
    return *this;
}
FrameUpdateSet::FrameUpdateSet(BOOST_RV_REF(FrameUpdateSet) other) :
        updateSet_()
{
    updateSet_.swap(other.updateSet_);
}
FrameUpdateSet& FrameUpdateSet::operator=(BOOST_RV_REF(FrameUpdateSet) other)
{
    updateSet_.swap(other.updateSet_);
    return *this;
}

void FrameUpdateSet::add(Frame* frame)
{
    assert(frame);
    updateSet_.insert(frame);
}
void FrameUpdateSet::add(FrameUpdateSet const& other)
{
    updateSet_.insert(
        other.updateSet_.begin(),
        other.updateSet_.end());
}
void FrameUpdateSet::swap(FrameUpdateSet& other)
{
    updateSet_.swap(other.updateSet_);
}
FrameUpdateSet::const_iterator
    FrameUpdateSet::begin() const {
    return updateSet_.begin();
}
FrameUpdateSet::const_iterator
    FrameUpdateSet::end() const {
    return updateSet_.end();
}
FrameUpdateSet::iterator
    FrameUpdateSet::begin() {
    return updateSet_.begin();
}
FrameUpdateSet::iterator
    FrameUpdateSet::end() {
    return updateSet_.end();
}
std::size_t FrameUpdateSet::size() const {
    return updateSet_.size();
}
bool FrameUpdateSet::empty() const {
    return updateSet_.empty();
}
bool operator==(
    FrameUpdateSet const& lhs,
    FrameUpdateSet const& rhs)
{
    return boost::equal(lhs,rhs);
}
}
