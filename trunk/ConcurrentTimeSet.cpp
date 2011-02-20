#include "ConcurrentTimeSet.h"
namespace hg {
ConcurrentTimeSet::ConcurrentTimeSet() :
set_()
{
}
void ConcurrentTimeSet::add(Frame* toAdd)
{
    set_.insert(SetType::value_type(toAdd, ConcurrentTimeSet::Empty()));
}
void ConcurrentTimeSet::remove(Frame* toRemove)
{
    set_.erase(toRemove);
}
}