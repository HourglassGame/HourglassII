#include "Frame.h"
#include "FrameUpdateSet.h"
#include "UniverseID.h"
#include "Universe.h"
#include <boost/range/adaptor/map.hpp>
#include <boost/range/adaptor/indirected.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range.hpp>
#include <functional>
#include <utility>
#include <cassert>

namespace hg {
Frame::Frame(int frameNumber, Universe &universe):
        frameNumber(frameNumber),
        universe(&universe),
        departures(),
        arrivals(),
        view()
{
}

void Frame::correctUniverse(Universe_Frame_access, Universe &newUniverse) noexcept
{
    universe = &newUniverse;
}

void Frame::correctDepartureFramePointers(Universe_Frame_access, FramePointerUpdater const &updater) {
    FrameDeparturesT updatedDepartures;
    for (auto &departurePair: departures) {
        updatedDepartures[updater.updateFrame(departurePair.first)] = std::move(departurePair.second);
    }
    departures.swap(updatedDepartures);
}
void Frame::correctArrivalFramePointers(Universe_Frame_access, FramePointerUpdater const &updater) {
    FrameArrivalsT updatedArrivals;
    for (auto &arrivalPair: arrivals) {
        bool inserted = updatedArrivals.insert(std::make_pair(
                            updater.updateFrame(arrivalPair.first), arrivalPair.second));
        assert(inserted);
    }
    arrivals.swap(updatedArrivals);
}
void Frame::correctArrivalObjectListPointers(Universe_Frame_access) {
    for (auto &arrivalPair: arrivals) {
        if (arrivalPair.first) {
            auto it = arrivalPair.first->departures.find(this);
            assert(it != arrivalPair.first->departures.end());
            arrivalPair.second = &it->second;
        }
    }
}

Frame const *Frame::nextFrame(TimeDirection direction) const {
    assert(direction != TimeDirection::INVALID);
    return nextFrameInSameUniverse(direction) ? getArbitraryFrame(*universe, frameNumber + direction) : nullptr;
}

Frame *Frame::nextFrame(TimeDirection direction) {
    assert(direction != TimeDirection::INVALID);
    return nextFrameInSameUniverse(direction) ? getArbitraryFrame(*universe, frameNumber + direction) : nullptr;
}

bool Frame::nextFrameInSameUniverse(TimeDirection direction) const {
    return (frameNumber != 0 && direction == TimeDirection::REVERSE)
            || (frameNumber != getTimelineLength(*universe) - 1 && direction == TimeDirection::FORWARDS);
}

Universe const &Frame::getUniverse() const {
    return *universe;
}
Universe &Frame::getUniverse() {
    return *universe;
}
int Frame::getFrameNumber() const {
    return frameNumber;
}

Frame const *nextFrame(Frame const *frame, TimeDirection direction)
{
    assert(frame);
    return frame->nextFrame(direction);
}
Frame *nextFrame(Frame *frame, TimeDirection direction)
{
    assert(frame);
    return frame->nextFrame(direction);
}
bool nextFrameInSameUniverse(Frame const *frame, TimeDirection direction) {
    assert(frame);
    return frame->nextFrameInSameUniverse(direction);
}
Universe const &getUniverse(Frame const *frame) {
    assert(frame);
    return frame->getUniverse();
}
Universe &getUniverse(Frame *frame) {
    assert(frame);
    return frame->getUniverse();
}
int getFrameNumber(Frame const *frame) {
    assert(frame);
    return frame->getFrameNumber();
}
bool isNullFrame(Frame const *frame) {
    return !frame;
}

namespace {
    auto FrameNotNull = [](Frame::FrameDeparturesT::value_type const &pair) {
        return !isNullFrame(pair.first);
    };
}

FrameUpdateSet Frame::updateDeparturesFromHere(FrameDeparturesT &&newDeparture)
{
    FrameUpdateSet changedTimes;

    //filter so that things can safely depart to the null frame
    //such departures do not create arrivals or updated frames, but are saved as departures.
    auto newDepartureFiltered(newDeparture | boost::adaptors::filtered(FrameNotNull));
    auto oldDepartureFiltered(departures   | boost::adaptors::filtered(FrameNotNull));

    auto       ni  (boost::begin(newDepartureFiltered));
    auto const nend(boost::  end(newDepartureFiltered));

    auto       oi  (boost::begin(oldDepartureFiltered));
    auto const oend(boost::  end(oldDepartureFiltered));

    auto removeOldArrival = [&] {
        oi->first->clearArrival(this);
        changedTimes.add(oi->first);
        ++oi;
    };
    auto addNewArrival = [&] {
        ni->first->insertArrival(std::make_pair(this, &ni->second));
        changedTimes.add(ni->first);
        ++ni;
    };
    auto modifyArrival = [&] {
        //Change even if arrivals are identical,
        //because old arrival pointer will become invalid
        ni->first->changeArrival(std::make_pair(this, &ni->second));
        if (ni->second != oi->second) changedTimes.add(ni->first);
        ++oi;
        ++ni;
    };

    auto checkOICompletion = [&] {
        if (oi == oend) {
            while (ni != nend) addNewArrival();
            return true;
        }
        return false;
    };
    auto checkNICompletion = [&] {
        if (ni == nend) {
            while (oi != oend) removeOldArrival();
            return true;
        }
        return false;
    };

    if (!(checkOICompletion() || checkNICompletion())) {
        while (true) {
            if (oi->first < ni->first) {
                removeOldArrival();
                if (checkOICompletion()) break;
            }
            else if (ni->first < oi->first) {
                addNewArrival();
                if (checkNICompletion()) break;
            }
            else {
                assert(oi->first == ni->first);
                modifyArrival();
                if (checkOICompletion() || checkNICompletion()) break;
            }
        }
    }

    departures.swap(newDeparture);
    //This code is an attempt to put the deletion of the
    //old departures_ into the parallel region of the program's
    //execution.
    FrameDeparturesT().swap(newDeparture);
    return changedTimes;
}

ObjectPtrList<Normal> Frame::getPrePhysics() const
{
    ObjectPtrList<Normal> retv;
    for (ObjectList<Normal> const &value:
            arrivals
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
    ObjectPtrList<Normal> retv;
    for (ObjectList<Normal> const &value:
            departures | boost::adaptors::map_values)
    {
        retv.add(value);
    }
    retv.sort();
    return retv;
}

void Frame::setPermanentArrival(ObjectList<Normal> const *newPermanentArrival) {
    //tbb::concurrent_hash_map doesn't supply a function that either changes
    //or inserts a mapping depending on whether the given key is already in the map
    //(like std::map::operator[]), so we implement our own.
    auto toSet = std::make_pair(nullptr, newPermanentArrival);
    bool didInsert(arrivals.insert(toSet));
    if (!didInsert) {
        FrameArrivalsT::accessor access;
        if (arrivals.find(access, toSet.first)) {
            access->second = toSet.second;
            return;
        }
        assert(false);
    }
}

void Frame::insertArrival(
    FrameArrivalsT::value_type const &toInsert)
{
    bool didInsert(arrivals.insert(toInsert));
    (void) didInsert;
    assert(didInsert && "Should only call insert when the element does not exist");
}
void Frame::changeArrival(
    FrameArrivalsT::value_type const &toChange)
{
    FrameArrivalsT::accessor access;
    if (arrivals.find(access, toChange.first)) {
        access->second = toChange.second;
        return;
    }
    assert(false && "Should only call change when the element does exist");
}
void Frame::clearArrival(Frame const *toClear)
{
    bool didErase(arrivals.erase(toClear));
    (void) didErase;
    assert(didErase && "Should only call Erase when the element does exist");
}
}
