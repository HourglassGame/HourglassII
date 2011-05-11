#include "Frame.h"
#include "FrameUpdateSet.h"
#include "UniverseID.h"
#include "Universe.h"
#include <boost/foreach.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range.hpp>
#include <functional>
#include <utility>
#include <cassert>
#define foreach BOOST_FOREACH
namespace hg {
Frame::Frame(std::size_t frameNumber, Universe& universe):
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
Frame* Frame::nextFrame(TimeDirection direction) const {
    assert(direction != INVALID);
    if (nextFrameInSameUniverse(direction)) {
        return universe_.getArbitraryFrame(frameNumber_ + direction);
    }
    else {
        Frame* parent(universe_.getInitiatorFrame());
        return parent ? parent->nextFrame(direction) : 0;
    }
}
bool Frame::nextFrameInSameUniverse(TimeDirection direction) const {
    return (frameNumber_ != 0 && direction == REVERSE)
            || (frameNumber_ != universe_.getTimelineLength() - 1 && direction == FORWARDS);
}
unsigned int Frame::nextFramePauseLevelDifference(TimeDirection direction) const {
    assert(direction != INVALID);
    return nextFramePauseLevelDifferenceAux(direction, 0);
}

Universe& Frame::getSubUniverse(PauseInitiatorID const& initiatorID) {
    SubUniverseMap::iterator it(subUniverses_.find(initiatorID));
    if (it == subUniverses_.end())
    {
        it = subUniverses_.insert(SubUniverseMap::value_type(initiatorID, Universe())).first;
        it->second.construct(this, initiatorID.timelineLength_, it->first);
    }
    return it->second;
}

unsigned int Frame::nextFramePauseLevelDifferenceAux(TimeDirection direction, unsigned int accumulator) const
{
    if (nextFrameInSameUniverse(direction)) {
        return accumulator;
    }
    else {
        ++accumulator;
        Frame* parent(universe_.getInitiatorFrame());
        return parent ? parent->nextFramePauseLevelDifferenceAux(direction, accumulator) : accumulator;
    }
}
Universe& Frame::getUniverse() const {
    return universe_;
}
std::size_t Frame::getFrameNumber() const {
    return frameNumber_;
}


bool nextFrameInSameUniverse(Frame const* frame, TimeDirection direction) {
    assert(frame);
    return frame->nextFrameInSameUniverse(direction);
}
unsigned int nextFramePauseLevelDifference(Frame const* frame, TimeDirection direction) {
    assert(frame);
    return frame->nextFramePauseLevelDifference(direction);
}
Universe& getUniverse(Frame const* frame) {
    assert(frame);
    return frame->getUniverse();
}
Universe& getSubUniverse(Frame* frame, PauseInitiatorID const& initiatorID)
{
    assert(frame);
    return frame->getSubUniverse(initiatorID);
}
std::size_t getFrameNumber(Frame const* frame) {
    assert(frame);
    return frame->getFrameNumber();
}
bool isNullFrame(Frame const* frame) {
    return !frame;
}

namespace {
    struct FrameNotNull : std::unary_function<const std::pair<Frame*, ObjectList>&, bool> {
        bool operator()(const std::pair<Frame*, ObjectList>& pair) const {
            return pair.first;
        }
    };
}

FrameUpdateSet Frame::updateDeparturesFromHere(std::map<Frame*, ObjectList>& newDeparture)
{
    FrameUpdateSet changedTimes;

    //filter so that things can safely depart to the null frame
    //such departures do not create arrivals or updated frames, but are saved as departures.
    typedef boost::filtered_range<FrameNotNull, std::map<Frame*, ObjectList> > filtered_range_t;

    filtered_range_t newDepartureFiltered(newDeparture | boost::adaptors::filtered(FrameNotNull()));
    filtered_range_t oldDepartureFiltered(departures_ | boost::adaptors::filtered(FrameNotNull()));    
    
    boost::range_iterator<filtered_range_t>::type ni(boost::begin(newDepartureFiltered));
    const boost::range_iterator<filtered_range_t>::type nend(boost::end(newDepartureFiltered));
    
    
    boost::range_iterator<filtered_range_t>::type oi(boost::begin(oldDepartureFiltered));
    const boost::range_iterator<filtered_range_t>::type oend(boost::end(oldDepartureFiltered));

    while (oi != oend) {
        while (true) {
            if (ni != nend) {
                if (ni->first < oi->first) {
                    ni->first->insertArrival(std::make_pair(this, &ni->second));
                    changedTimes.add(ni->first);
                    ++ni;
                }
                else if (ni->first == oi->first) {
                    if (ni->second != oi->second) {
                        changedTimes.add(ni->first);
                    }
                    //Always change because old pointer will become invalid
                    ni->first->changeArrival(std::make_pair(this, &ni->second));
                    ++ni;
                    break;
                }
                else {
                    oi->first->clearArrival(this);
                    changedTimes.add(oi->first);
                    break;
                }
            }
            else {
                while (oi != oend) {
                    oi->first->clearArrival(this);
                    changedTimes.add(oi->first);
                    ++oi;
                }
                goto end;
            }
        }
        ++oi;
    }
    while (ni != nend) {
        ni->first->insertArrival(std::make_pair(this, &ni->second));
        changedTimes.add(ni->first);
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
    foreach (const ArrivalMap::value_type& value, arrivals_) {
        retv.add(*value.second);
    }
    retv.sort();
    return retv;
}
ObjectPtrList Frame::getPostPhysics(/*const PauseInitiatorID& whichPrePause*/) const
{
    ObjectPtrList retv;
    typedef std::map<Frame*, ObjectList>::value_type value_type;
    foreach (const value_type& value, departures_) {
        retv.add(value.second);
    }
    retv.sort();
    return retv;
}

void Frame::addArrival(Frame* source, ObjectList* arrival)
{
    arrivals_.insert(ArrivalMap::value_type(source, arrival));
}
void Frame::insertArrival(const ArrivalMap::value_type& toInsert)
{
    bool didInsert(arrivals_.insert(toInsert));
    (void) didInsert;
    assert(didInsert && "Should only call insert when the element does not exist");
}
void Frame::changeArrival(const ArrivalMap::value_type& toChange)
{
    ArrivalMap::accessor access;
    if (arrivals_.find(access, toChange.first))
    {
        access->second = toChange.second;
        return;
    }
    assert(false && "Should only call change when the element does exist");
}
void Frame::clearArrival(Frame* toClear)
{
    bool didErase(arrivals_.erase(toClear));
    (void) didErase;
    assert(didErase && "Should only call Erase when the element does exist");
}

Frame* nextFrame(Frame const* frame, TimeDirection direction)
{
    assert(frame);
    return frame->nextFrame(direction);
}
}
