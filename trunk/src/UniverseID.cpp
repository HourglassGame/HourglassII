#include "UniverseID.h"
#include "FrameID.h"
#include "Frame.h"

#include <boost/functional/hash.hpp>
#include <boost/range/algorithm/reverse.hpp>

#include "Universe.h"
namespace hg {
UniverseID::UniverseID(std::size_t timelineLength) :
    timelineLength_(timelineLength)
{
}

std::size_t UniverseID::timelineLength() const
{
    return timelineLength_;
}

bool UniverseID::operator==(UniverseID const& other) const
{
    return timelineLength_ == other.timelineLength_;
}

bool UniverseID::operator<(UniverseID const& other) const
{
    return timelineLength_ < other.timelineLength_;
}

FrameID getEntryFrame(UniverseID const& universe, TimeDirection direction)
{
    assert(getTimelineLength(universe));
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
FrameID getArbitraryFrame(UniverseID const& universe, std::size_t frameNumber)
{
    return frameNumber < getTimelineLength(universe) ? FrameID(frameNumber, universe) : FrameID();
}
std::size_t getTimelineLength(UniverseID const& universe)
{
    return universe.timelineLength_;
}

std::size_t hash_value(UniverseID const& toHash)
{
    std::size_t seed(0);
    boost::hash_combine(seed, toHash.timelineLength_);
    return seed;
}
}//namespace hg

