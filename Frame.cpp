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
        editDepartures_(),
        editArrivals_(),
        subUniverses_()
{
}
//copy-construction
Frame::Frame(const Frame& other) :
        frameNumber_(other.frameNumber_),
        universe_(other.universe_),
        departures_(other.departures_),
        arrivals_(other.arrivals_),
        editDepartures_(other.editDepartures_),
        editArrivals_(other.editArrivals_),
        subUniverses_(other.subUniverses_)
{
}
Frame const* Frame::nextFrame(TimeDirection direction) const {
    assert(direction != INVALID);
    if (nextFrameInSameUniverse(direction)) {
        return universe_.getArbitraryFrame(frameNumber_ + direction);
    }
    else {
        Frame* parent(universe_.getInitiatorFrame());
        return parent ? parent->nextFrame(direction) : 0;
    }
}

Frame* Frame::nextFrame(TimeDirection direction) {
    //these const_casts are well defined as frames are always in universes,
    //and their underlying objects are always non-const.
    return const_cast<Frame*>(const_cast<Frame const*>(this)->nextFrame(direction));
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
Universe const& Frame::getUniverse() const {
    return universe_;
}
Universe& Frame::getUniverse() {
    return universe_;
}
std::size_t Frame::getFrameNumber() const {
    return frameNumber_;
}
boost::select_second_const_range<Frame::SubUniverseMap>
    Frame::getSubUniverseList() const
{
    return boost::adaptors::values(subUniverses_);
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
unsigned int nextFramePauseLevelDifference(Frame const* frame, TimeDirection direction) {
    assert(frame);
    return frame->nextFramePauseLevelDifference(direction);
}
Universe const& getUniverse(Frame const* frame) {
    assert(frame);
    return frame->getUniverse();
}
Universe& getUniverse(Frame * frame) {
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
boost::select_second_const_range<Frame::SubUniverseMap>
       getSubUniverseList(Frame const* frame)
{
    assert(frame);
    return frame->getSubUniverseList();
}
namespace {
unsigned getPauseDepth(Frame const* frame) {
    assert(frame);
    unsigned depth(0);
    while ((frame = getInitiatorFrame(getUniverse(frame)))) {
        ++depth;
    }
    return depth;
}
int compare(std::size_t l, std::size_t r, TimeDirection direction)
{
    assert(direction != INVALID);
    assert(l != std::numeric_limits<std::size_t>::max());
    assert(r != std::numeric_limits<std::size_t>::max());
    if (l < r) {
        return -direction;
    }
    else if (l > r) {
        return direction;
    }
    return 0;
}
int compare(PauseInitiatorID const& l, PauseInitiatorID const& r, TimeDirection direction)
{
    assert(direction != INVALID);
    if (l < r) {
        return -direction;
    }
    else if (l > r) {
        return direction;
    }
    return 0;
}
}
int compare(Frame const* l, Frame const* r, TimeDirection direction)
{
    assert(l);
    assert(r);
    assert(direction != INVALID);
    unsigned thisDepth(getPauseDepth(l));
    unsigned otherDepth(getPauseDepth(r));
    
    if (thisDepth > otherDepth) {
        return compare(getInitiatorFrame(getUniverse(l)), r, direction);
    }
    else if (otherDepth > thisDepth) {
        return compare(l, getInitiatorFrame(getUniverse(r)), direction);
    }
    int parentComparison(compare(getInitiatorFrame(getUniverse(l)), getInitiatorFrame(getUniverse(r)), direction));
    if (parentComparison) {
        return parentComparison;
    }
    Frame const* lParent(getInitiatorFrame(getUniverse(l)));
#ifndef NDEBUG
    Frame const* rParent(getInitiatorFrame(getUniverse(l)));
    assert(lParent == rParent);
#endif
    if (lParent) {
        int universeComparison(compare(getInitiatorID(getUniverse(l)), getInitiatorID(getUniverse(r)), direction));
        if (universeComparison) {
            return universeComparison;
        }
    }
    return compare(getFrameNumber(l), getFrameNumber(r), direction);
}
bool isNullFrame(Frame const* frame) {
    return !frame;
}

namespace {
    template<typename ListTypes>
    struct FrameNotNull : std::unary_function<const std::pair<Frame*, ObjectList<ListTypes> >&, bool> {
        bool operator()(const typename std::pair<Frame*, ObjectList<ListTypes> >& pair) const {
            return !isNullFrame(pair.first);
        }
    };
}

void Frame::setRawDepartures(std::map<Frame*, ObjectList<Normal> >& newDeparture)
{
    rawDepartures_.swap(newDeparture);
}
std::map<Frame*, ObjectList<Normal> > const& Frame::getRawDepartures() const
{
    return rawDepartures_;
}
FrameUpdateSet Frame::updateDeparturesFromHere(std::map<Frame*, ObjectList<Normal> >& newDeparture)
{
    //Big code duplication between this an updateDeparturesFromHere... Fix at some time.
    FrameUpdateSet changedTimes;

    //filter so that things can safely depart to the null frame
    //such departures do not create arrivals or updated frames, but are saved as departures.
    typedef boost::filtered_range<FrameNotNull<Normal>, std::map<Frame*, ObjectList<Normal> > > filtered_range_t;

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
    std::map<Frame*, ObjectList<Normal> > deleter;
    deleter.swap(newDeparture);
    return changedTimes;
}

FrameUpdateSet Frame::updateEditDeparturesFromHere(std::map<Frame*, ObjectList<FirstEdit> >& newDeparture)
{
    FrameUpdateSet changedTimes;

    //filter so that things can safely depart to the null frame
    //such departures do not create arrivals or updated frames, but are saved as departures.
    typedef boost::filtered_range<FrameNotNull<FirstEdit>, std::map<Frame*, ObjectList<FirstEdit> > > filtered_range_t;

    filtered_range_t newDepartureFiltered(newDeparture | boost::adaptors::filtered(FrameNotNull<FirstEdit>()));
    filtered_range_t oldDepartureFiltered(editDepartures_ | boost::adaptors::filtered(FrameNotNull<FirstEdit>()));    
    
    boost::range_iterator<filtered_range_t>::type ni(boost::begin(newDepartureFiltered));
    const boost::range_iterator<filtered_range_t>::type nend(boost::end(newDepartureFiltered));
    
    
    boost::range_iterator<filtered_range_t>::type oi(boost::begin(oldDepartureFiltered));
    const boost::range_iterator<filtered_range_t>::type oend(boost::end(oldDepartureFiltered));

    while (oi != oend) {
        while (true) {
            if (ni != nend) {
                if (ni->first < oi->first) {
                    ni->first->insertEditArrival(std::make_pair(this, &ni->second));
                    changedTimes.add(ni->first);
                    ++ni;
                }
                else if (ni->first == oi->first) {
                    if (ni->second != oi->second) {
                        changedTimes.add(ni->first);
                    }
                    //Always change because old pointer will become invalid
                    ni->first->changeEditArrival(std::make_pair(this, &ni->second));
                    ++ni;
                    break;
                }
                else {
                    oi->first->clearEditArrival(this);
                    changedTimes.add(oi->first);
                    break;
                }
            }
            else {
                while (oi != oend) {
                    oi->first->clearEditArrival(this);
                    changedTimes.add(oi->first);
                    ++oi;
                }
                goto end;
            }
        }
        ++oi;
    }
    while (ni != nend) {
        ni->first->insertEditArrival(std::make_pair(this, &ni->second));
        changedTimes.add(ni->first);
        ++ni;
    }
end:
    editDepartures_.swap(newDeparture);
    std::map<Frame*, ObjectList<FirstEdit> > deleter;
    deleter.swap(newDeparture);
    return changedTimes;
}


ObjectPtrList<Normal>  Frame::getPrePhysics() const
{
    ObjectPtrList<Normal>  retv;
    typedef tbb::concurrent_hash_map<Frame const*, ObjectList<Normal> const*>::value_type value_t;
    foreach (const value_t& value, arrivals_) {
        retv.add(*value.second);
    }
    retv.sort();
    return retv;
}

ObjectPtrList<FirstEdit>  Frame::getPreEdits() const
{
    ObjectPtrList<FirstEdit>  retv;
    typedef tbb::concurrent_hash_map<Frame const*, ObjectList<FirstEdit> const*>::value_type value_t;
    foreach (const value_t& value, editArrivals_) {
        retv.add(*value.second);
    }
    retv.sort();
    return retv;
}

ObjectPtrList<Normal>  Frame::getPostPhysics(/*const PauseInitiatorID& whichPrePause*/) const
{
    ObjectPtrList<Normal>  retv;
    typedef std::map<Frame*, ObjectList<Normal> >::value_type value_type;
    foreach (const value_type& value, departures_) {
            retv.add(value.second);
    }
    //sort() triggers assert on activation of pause time because
    //the combined departures contain both the normal and pause departures
    //(of buttons for example), which both have the same index.
    //This is a real issue, but I would like to wait on fixing it until 
    //the new pause time is implemented. So the sort is commented out.
    //TODO - proper investigations.
    //retv.sort();
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

void Frame::insertEditArrival(const tbb::concurrent_hash_map<Frame const*, ObjectList<FirstEdit> const*>::value_type& toInsert)
{
    bool didInsert(editArrivals_.insert(toInsert));
    (void) didInsert;
    assert(didInsert && "Should only call insert when the element does not exist");
}
void Frame::changeEditArrival(const tbb::concurrent_hash_map<Frame const*, ObjectList<FirstEdit> const*>::value_type& toChange)
{
    tbb::concurrent_hash_map<Frame const*, ObjectList<FirstEdit> const*>::accessor access;
    if (editArrivals_.find(access, toChange.first))
    {
        access->second = toChange.second;
        return;
    }
    assert(false && "Should only call change when the element does exist");
}
void Frame::clearEditArrival(Frame const* toClear)
{
    bool didErase(editArrivals_.erase(toClear));
    (void) didErase;
    assert(didErase && "Should only call Erase when the element does exist");
}
}
