#include "FrameUpdateSet.h"
#include <boost/range/algorithm/equal.hpp>
namespace hg {
FrameUpdateSet::FrameUpdateSet() :
updateSet_()
{
}
void FrameUpdateSet::add(Frame* frame)
{ 
    if (frame) {
        updateSet_.insert(frame);
    }
}
void FrameUpdateSet::add(const FrameUpdateSet& other)
{
    updateSet_.insert(other.updateSet_.begin(), other.updateSet_.end());
}
void FrameUpdateSet::swap(FrameUpdateSet& other)
{
    updateSet_.swap(other.updateSet_);
}
FrameUpdateSet::const_iterator FrameUpdateSet::begin() const { return updateSet_.begin(); }
FrameUpdateSet::const_iterator FrameUpdateSet::end() const { return updateSet_.end(); }
FrameUpdateSet::iterator FrameUpdateSet::begin() { return updateSet_.begin(); }
FrameUpdateSet::iterator FrameUpdateSet::end() { return updateSet_.end(); }
std::size_t FrameUpdateSet::size() const { return updateSet_.size(); }
bool FrameUpdateSet::empty() const { return updateSet_.empty(); }
bool operator==(const FrameUpdateSet& lhs, const FrameUpdateSet& rhs)
{
    return boost::equal(lhs,rhs);
}
}
