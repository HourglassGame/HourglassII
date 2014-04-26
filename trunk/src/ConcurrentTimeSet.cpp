#include "ConcurrentTimeSet.h"
namespace hg {
void ConcurrentTimeSet::add(Frame *toAdd)
{
    set.insert(SetType::value_type(toAdd, ConcurrentTimeSet::Empty()));
}
void ConcurrentTimeSet::remove(Frame *toRemove)
{
    set.erase(toRemove);
}
}
