#include "FrameID.h"

#include "Frame.h"

#include <boost/functional/hash.hpp>
#include <boost/tuple/tuple.hpp>

#include <limits>
#include <cassert>

namespace hg {
//<Private member functions>
namespace {
    struct NextFrameInSameUniverse {
        typedef bool Return;
        struct ArgT {
            ArgT(const FrameID& this__,
                TimeDirection direction_) :
                this_(this__), direction(direction_){}
            const FrameID& this_;
            TimeDirection direction;};
        typedef const ArgT& Args;};}
template<>
bool FrameID::p<NextFrameInSameUniverse>(NextFrameInSameUniverse::Args args);

namespace {
    struct NextFramePauseLevelDifferenceAux {
        typedef unsigned int Return;
        struct ArgT {
            ArgT(const FrameID& this__,
            unsigned accumulator_,
            TimeDirection direction_) :
            this_(this__),
            accumulator(accumulator_),
            direction(direction_) {}
            const FrameID& this_;
            unsigned accumulator;
            TimeDirection direction;};
        typedef const ArgT& Args; };}
template<> unsigned FrameID::p<NextFramePauseLevelDifferenceAux>(
    NextFramePauseLevelDifferenceAux::Args args);
//</private member functions>

FrameID::FrameID() :
        frame_(std::numeric_limits<std::size_t>::max()),
        universeID_(0)
{
}
FrameID::FrameID(std::size_t time, const UniverseID& nuniverse) :
        frame_(time),
        universeID_(nuniverse)
{
    assert(isValidFrame());
}
//Creates a FrameID corresponding to the given Frame*
FrameID::FrameID(const Frame* toConvert) :
        frame_(toConvert->frameNumber_),
        universeID_(toConvert->universe_)
{
}
FrameID FrameID::nextFrame(TimeDirection direction) const
{
    return 
    isValidFrame() ? 
        (p<NextFrameInSameUniverse>(NextFrameInSameUniverse::ArgT(*this, direction)) ? 
            FrameID(frame_ + direction, universeID_) :
            universeID_.parentFrame().nextFrame(direction)) :
        FrameID();
}
unsigned int FrameID::nextFramePauseLevelDifference(TimeDirection direction) const
{
    assert(isValidFrame());
    return p<NextFramePauseLevelDifferenceAux>(NextFramePauseLevelDifferenceAux::ArgT(*this, 0u, direction));
}
template<> unsigned FrameID::p<NextFramePauseLevelDifferenceAux>(
    NextFramePauseLevelDifferenceAux::Args a)
{
    return 
           !a.this_.isValidFrame()
        || p<NextFrameInSameUniverse>(NextFrameInSameUniverse::ArgT(a.this_,a.direction)) ?
              a.accumulator :
              p<NextFramePauseLevelDifferenceAux>(
                  NextFramePauseLevelDifferenceAux::ArgT(
                    a.this_.universeID_.parentFrame(),
                    a.accumulator + 1,
                    a.direction));
}
template<>
bool FrameID::p<NextFrameInSameUniverse>(NextFrameInSameUniverse::Args a)
{
    return
        assert(a.this_.isValidFrame()),
        (a.direction == REVERSE && a.this_.frame_ != 0)
     || (a.direction == FORWARDS && a.this_.frame_ != a.this_.universeID_.timelineLength() - 1);
}
FrameID FrameID::arbitraryFrameInUniverse(std::size_t frameNumber) const
{
    return frameNumber < universeID_.timelineLength() ?
        FrameID(frameNumber, universeID_) : FrameID();
}
FrameID FrameID::parentFrame() const
{
    return universeID_.parentFrame();
}
FrameID FrameID::arbitraryChildFrame(
    const PauseInitiatorID& initatorID,
    std::size_t frameNumber) const
{
    return
    FrameID(
        frameNumber,
        universeID_.getSubUniverse(SubUniverse(frame_, initatorID)));
}
FrameID FrameID::entryChildFrame(
    const PauseInitiatorID& initatorID,
    TimeDirection direction) const
{
    assert(initatorID.timelineLength_ != 0);
    return FrameID(direction == FORWARDS ? 0 : initatorID.timelineLength_ - 1,
                   universeID_.getSubUniverse(SubUniverse(frame_, initatorID)));
}
bool FrameID::operator==(const FrameID& other) const {
    return frame_ == other.frame_ && universeID_ == other.universeID_;
}
bool FrameID::operator<(const FrameID& other) const {
    return
    universeID_ == other.universeID_ ?
        frame_ < other.frame_ :
        universeID_ < other.universeID_;
}
bool FrameID::isValidFrame() const {
    return
    frame_ < universeID_.timelineLength() ?
        true :
        (assert(frame_ == std::numeric_limits<std::size_t>::max()),
        assert(universeID_.timelineLength() == 0),
        false);
}
std::size_t FrameID::getFrameNumber() const {
    assert (isValidFrame());
    return frame_;
}
const UniverseID& FrameID::universe() const {
    return universeID_;
}
std::size_t hash_value(const FrameID& toHash) {
    std::size_t seed(0);
    return
    toHash.isValidFrame() ?
        seed :
        (boost::hash_combine(seed, toHash.getFrameNumber()),
        boost::hash_combine(seed, toHash.universe()),
        seed);
}
}
