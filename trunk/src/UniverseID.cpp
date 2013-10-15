#include "UniverseID.h"
#include "FrameID.h"
#include "Frame.h"

#include <boost/functional/hash.hpp>
#include <boost/range/algorithm/reverse.hpp>

#include "Universe.h"
namespace hg {
UniverseID::UniverseID(int timelineLength) :
    timelineLength_(timelineLength)
{
}

int UniverseID::timelineLength() const
{
    return timelineLength_;
}

bool UniverseID::operator==(UniverseID const &o) const
{
    return timelineLength_ == o.timelineLength_;
}

bool UniverseID::operator<(UniverseID const &o) const
{
    return timelineLength_ < o.timelineLength_;
}

FrameID getEntryFrame(UniverseID const &universe, TimeDirection direction)
{
    assert(getTimelineLength(universe) >= 0);
    switch (direction) {
    case FORWARDS:
        return FrameID(0, universe);
    case REVERSE:
        return FrameID(getTimelineLength(universe) - 1, universe);
    default:
        assert(false);
    }
    //Never reached
    return FrameID();
}
FrameID getArbitraryFrame(UniverseID const &universe, int frameNumber)
{
    assert(getTimelineLength(universe) >= 0);
    return frameNumber < getTimelineLength(universe) && frameNumber >= 0 ? FrameID(frameNumber, universe) : FrameID();
}
int getTimelineLength(UniverseID const &universe)
{
    return universe.timelineLength_;
}

std::size_t hash_value(UniverseID const &toHash)
{
    std::size_t seed(0);
    boost::hash_combine(seed, toHash.timelineLength_);
    return seed;
}
}//namespace hg

