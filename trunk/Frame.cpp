#include "Frame.h"
#include "FrameUpdateSet.h"
#include <boost/foreach.hpp>
#include <utility>
#include <cassert>
#define foreach BOOST_FOREACH
namespace hg {
Frame::Frame(unsigned int frameNumber, Universe* universe):
frameNumber_(frameNumber),
universe_(universe),
departures_(),
arrivals_(),
subUniverses_()
{
}
//copy-construction
Frame::Frame(const Frame& other) :
frameNumber_(other.frameNumber_),
universe_(other.universe_),
departures_(other.departures_),
arrivals_(other.arrivals_),
subUniverses_(other.subUniverses_)
{
}
Frame* Frame::nextFrame(TimeDirection direction) const
{
    assert(direction != INVALID);
    if (frameNumber_ == 0 && direction == REVERSE
     || frameNumber_ == universe_->getTimelineLength() - 1 && direction == FORWARDS) {
        Frame* parent(universe_->getInitiatorFrame());
        return parent ? parent->nextFrame(direction) : 0;
    }
    else {
        return universe_->getArbitraryFrame(frameNumber_ + direction);
    }
}
unsigned int Frame::nextFramePauseLevelDifference(TimeDirection direction) const
{
    assert(direction != INVALID);
    return nextFramePauseLevelDifferenceAux(direction, 0);
}
Frame* Frame::arbitraryFrameInUniverse(unsigned int frameNumber) const
{
    return universe_->getArbitraryFrame(frameNumber);
}
Frame* Frame::parentFrame() const
{
    return universe_->getInitiatorFrame();
}
Frame* Frame::arbitraryChildFrame(const PauseInitiatorID& initiatorID, unsigned int frameNumber)
{
    assert(initiatorID.timelineLength_ > frameNumber);
    SubUniverseMap::iterator it(subUniverses_.find(initiatorID));
    if(it == subUniverses_.end())
    {
        it = subUniverses_.insert(SubUniverseMap::value_type(initiatorID, Universe())).first;
        it->second.construct(this, initiatorID.timelineLength_, it->first);
    }
    return it->second.getArbitraryFrame(frameNumber);
}
Frame* Frame::entryChildFrame(const PauseInitiatorID& initiatorID, TimeDirection direction)
{
    SubUniverseMap::iterator it(subUniverses_.find(initiatorID));
    if(it == subUniverses_.end())
    {
        it = subUniverses_.insert(SubUniverseMap::value_type(initiatorID, Universe())).first;
        it->second.construct(this, initiatorID.timelineLength_, it->first);
    }
    return it->second.getEntryFrame(direction);
}
FrameUpdateSet Frame::updateDeparturesFromHere(std::map<Frame*, ObjectList>& newDeparture)
{
    FrameUpdateSet changedTimes;

    std::map<Frame*, ObjectList>::iterator ni(newDeparture.begin());
    const std::map<Frame*, ObjectList>::iterator nend(newDeparture.end());
    std::map<Frame*, ObjectList>::iterator oi(departures_.begin());
    const std::map<Frame*, ObjectList>::iterator oend(departures_.end());

    while (oi != oend) {
        while (true) {
            if (ni != nend) {
                if (ni->first < oi->first) {
                    ni->first->insertArrival(std::make_pair(this, &ni->second));
                    changedTimes.addFrame(ni->first);
                    ++ni;
                }
                else if (ni->first == oi->first) {
                    if (ni->second != oi->second) {
                        changedTimes.addFrame(ni->first);
                    }
                    //Always change because old pointer will become invalid
                    ni->first->changeArrival(std::make_pair(this, &ni->second));
                    ++ni;
                    break;
                }
                else {
                    oi->first->clearArrival(this);
                    changedTimes.addFrame(oi->first);
                    break;
                }
            }
            else {
                while (oi != oend) {
                    oi->first->clearArrival(this);
                    changedTimes.addFrame(oi->first);
                    ++oi;
                }
                goto end;
            }
        }
        ++oi;
    }
    while (ni != nend) {
        ni->first->insertArrival(std::make_pair(this, &ni->second));
        changedTimes.addFrame(ni->first);
        ++ni;
    }
end:
    departures_.clear();
    departures_.swap(newDeparture);
    return changedTimes;
}
ObjectPtrList Frame::getPrePhysics() const
{
    ObjectPtrList retv;
    foreach(const ArrivalMap::value_type& value, arrivals_) {
        retv.add(*value.second);
    }
    retv.sort();
    return retv;
}
ObjectPtrList Frame::getPostPhysics(/*const PauseInitiatorID& whichPrePause*/) const
{
    ObjectPtrList retv;
    typedef std::map<Frame*, ObjectList>::value_type value_type;
    foreach(const value_type& value, departures_) {
        retv.add(value.second);
    }
    retv.sort();
    return retv;
}
const PauseInitiatorID& Frame::getInitiatorID() const {
    return universe_->getInitiatorID();
}
unsigned int Frame::nextFramePauseLevelDifferenceAux(TimeDirection direction, int accumulator) const
{
    if (frameNumber_ == 0 && direction == REVERSE
     || frameNumber_ == universe_->getTimelineLength() - 1 && direction == FORWARDS) {
        ++accumulator;
        Frame* parent(universe_->getInitiatorFrame());
        return parent ? parent->nextFramePauseLevelDifferenceAux(direction, accumulator) : accumulator;
    }
    else {
        return accumulator;
    }
}

void Frame::addArrival(Frame* source, ObjectList* arrival)
{
    arrivals_.insert(ArrivalMap::value_type(source, arrival));
}

void Frame::insertArrival(const ArrivalMap::value_type& toInsert)
{
    bool didInsert(arrivals_.insert(toInsert));
    assert(didInsert && "Should only call insert when the element does not exist");
}
void Frame::changeArrival(const ArrivalMap::value_type& toChange)
{
    ArrivalMap::accessor access;
    if(arrivals_.find(access, toChange.first))
    {
        access->second = toChange.second;
        return;
    }
    assert(false && "Should only call change when the element does exist");
}
void Frame::clearArrival(Frame* toClear)
{
    bool didErase(arrivals_.erase(toClear));
    assert(didErase && "Should only call Erase when the element does exist");
}
}
