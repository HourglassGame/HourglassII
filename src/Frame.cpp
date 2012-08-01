#include "Frame.h"
#include "FrameUpdateSet.h"
#include "UniverseID.h"
#include "Universe.h"
#include "Foreach.h"
#include <boost/range/adaptor/map.hpp>
#include <boost/range/adaptor/indirected.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range.hpp>
#include <functional>
#include <utility>
#include <cassert>

namespace hg {
Frame::Frame(int frameNumber, Universe& universe):
        frameNumber_(frameNumber),
        universe_(&universe),
        departures_(),
        arrivals_(),
        view_()
{
}

void Frame::correctUniverse(Universe& newUniverse)
{
    universe_ = &newUniverse;
}

Frame const* Frame::nextFrame(TimeDirection direction) const {
    assert(direction != INVALID);
    return nextFrameInSameUniverse(direction) ? universe_->getArbitraryFrame(frameNumber_ + direction) : 0;
}

Frame* Frame::nextFrame(TimeDirection direction) {
    assert(direction != INVALID);
    return nextFrameInSameUniverse(direction) ? universe_->getArbitraryFrame(frameNumber_ + direction) : 0;
}

bool Frame::nextFrameInSameUniverse(TimeDirection direction) const {
    return (frameNumber_ != 0 && direction == REVERSE)
            || (frameNumber_ != universe_->getTimelineLength() - 1 && direction == FORWARDS);
}

Universe const& Frame::getUniverse() const {
    return *universe_;
}
Universe& Frame::getUniverse() {
    return *universe_;
}
int Frame::getFrameNumber() const {
    return frameNumber_;
}

Frame const* nextFrame(Frame const* frame, TimeDirection direction)
{
    assert(frame);
    return frame->nextFrame(direction);
}
Frame* nextFrame(Frame* frame, TimeDirection direction)
{
    assert(frame);
    return frame->nextFrame(direction);
}
bool nextFrameInSameUniverse(Frame const* frame, TimeDirection direction) {
    assert(frame);
    return frame->nextFrameInSameUniverse(direction);
}
Universe const& getUniverse(Frame const* frame) {
    assert(frame);
    return frame->getUniverse();
}
Universe& getUniverse(Frame * frame) {
    assert(frame);
    return frame->getUniverse();
}
int getFrameNumber(Frame const* frame) {
    assert(frame);
    return frame->getFrameNumber();
}
bool isNullFrame(Frame const* frame) {
    return !frame;
}

namespace {
    template<typename ListTypes>
    struct FrameNotNull : std::unary_function<std::pair<Frame* const, ObjectList<ListTypes> > const&, bool> {
        bool operator()(Frame::FrameDeparturesT::value_type const& pair) const {
            return !isNullFrame(pair.first);
        }
    };
}

FrameUpdateSet Frame::updateDeparturesFromHere(FrameDeparturesT& newDeparture)
{
    FrameUpdateSet changedTimes;

    //filter so that things can safely depart to the null frame
    //such departures do not create arrivals or updated frames, but are saved as departures.
    typedef boost::filtered_range<FrameNotNull<Normal>, FrameDeparturesT> filtered_range_t;

    filtered_range_t newDepartureFiltered(newDeparture | boost::adaptors::filtered(FrameNotNull<Normal>()));
    filtered_range_t oldDepartureFiltered(departures_ | boost::adaptors::filtered(FrameNotNull<Normal>()));

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
    departures_.swap(newDeparture);
    //attempt to put the deletion of the old departures_
    //into the parallel region of the program's execution
    FrameDeparturesT().swap(newDeparture);
    return changedTimes;
}

ObjectPtrList<Normal> Frame::getPrePhysics() const
{
    ObjectPtrList<Normal>  retv;
    foreach (ObjectList<Normal> const& value,
    		arrivals_
            | boost::adaptors::map_values
            | boost::adaptors::indirected)
    {
        retv.add(value);
    }
    retv.sort();
    return retv;
}

ObjectPtrList<Normal> Frame::getPostPhysics() const
{
    ObjectPtrList<Normal>  retv;
    foreach (ObjectList<Normal> const& value,
    		departures_ | boost::adaptors::map_values)
    {
        retv.add(value);
    }
    retv.sort();
    return retv;
}

void Frame::addArrival(Frame const* source, ObjectList<Normal> const* arrival)
{
    insertArrival(tbb::concurrent_hash_map<Frame const*, ObjectList<Normal> const*>::value_type(source, arrival));
}
void Frame::insertArrival(const tbb::concurrent_hash_map<Frame const*, ObjectList<Normal> const*>::value_type& toInsert)
{
    bool didInsert(arrivals_.insert(toInsert));
    (void) didInsert;
    assert(didInsert && "Should only call insert when the element does not exist");
}
void Frame::changeArrival(const tbb::concurrent_hash_map<Frame const*, ObjectList<Normal> const*>::value_type& toChange)
{
    tbb::concurrent_hash_map<Frame const*, ObjectList<Normal> const*>::accessor access;
    if (arrivals_.find(access, toChange.first))
    {
        access->second = toChange.second;
        return;
    }
    assert(false && "Should only call change when the element does exist");
}
void Frame::clearArrival(Frame const* toClear)
{
    bool didErase(arrivals_.erase(toClear));
    (void) didErase;
    assert(didErase && "Should only call Erase when the element does exist");
}
}
