#ifndef HG_UNIVERSE_H
#define HG_UNIVERSE_H

#include "TimeDirection.h"
#include "ObjectList.h"
#include "ObjectListTypes.h"

#include "Universe_fwd.h"
#include "FrameID_fwd.h"
#include "Frame_fwd.h"

#include <boost/container/vector.hpp>
#include <type_traits>
namespace hg {
class Universe;
struct FramePointerUpdater {
    FramePointerUpdater(Universe& universe);
    template<typename FrameT>
    FrameT updateFrame(FrameT frame) const {
        return frame ? startOfUniverse + getFrameNumber(frame) : frame;
    }
private:
    Frame* startOfUniverse;
};


//Returns the first frame in the universe for objects traveling
//in TimeDirection direction.
Frame *getEntryFrame(Universe &universe, TimeDirection direction);
Frame const *getEntryFrame(Universe const &universe, TimeDirection direction);
//Returns the frame with the index frameNumber within the universe,
//or the NullFrame if no such frame exists
Frame *getArbitraryFrame(Universe &universe, int frameNumber);
Frame const *getArbitraryFrame(Universe const &universe, int frameNumber);
//Returns the frame with the index closest to frameNumber within the universe.
Frame *getArbitraryFrameClamped(Universe &universe, int frameNumber);
Frame const *getArbitraryFrameClamped(Universe const &universe, int frameNumber);
//returns the length of this universe's timeline
int getTimelineLength(Universe const &universe);

class Universe {
public:
    explicit Universe(int timelineLength);

    Universe(Universe const& o);
    Universe &operator=(Universe const& o);

    Universe(Universe&& o);
    Universe &operator=(Universe&& o);

    //Conversion from FrameID to equivalent Frame * within this universe
    //whichFrame must correspond to a frame that could actually
    //be in the universe.
    Frame *getFrame(FrameID const &whichFrame);
    Frame const *getFrame(FrameID const &whichFrame) const;

private:
    friend class Frame;
    friend class UniverseID;
    friend struct FramePointerUpdater;
    void fixFramesUniverses();
    void fixFramesEverything();
    //UniverseT interface {
    //Returns the first frame in the universe for objects traveling
    //in TimeDirection direction.
    Frame *getEntryFrame(TimeDirection direction);
    Frame const *getEntryFrame(TimeDirection direction) const;
    //Returns the frame with the index frameNumber within the universe,
    //or the NullFrame if no such frame exists
    Frame *getArbitraryFrame(int frameNumber);
    Frame const *getArbitraryFrame(int frameNumber) const;
    //Returns the frame with the index closest to frameNumber within the universe.
    Frame *getArbitraryFrameClamped(int frameNumber);
    Frame const *getArbitraryFrameClamped(int frameNumber) const;
    //returns the length of this Universe's timeline
    int getTimelineLength() const;
    //}
    
    template<typename UniverseT>
    using FrameMatchingUniverseConstness = typename std::conditional<std::is_const<UniverseT>::value, Frame const, Frame>::type;
    
    template<typename UniverseT>
    static FrameMatchingUniverseConstness<UniverseT> *getFrameImpl(UniverseT &universe, FrameID const &whichFrame);
    template<typename UniverseT>
    static FrameMatchingUniverseConstness<UniverseT> *getEntryFrameImpl(UniverseT &universe, TimeDirection direction);
    template<typename UniverseT>
    static FrameMatchingUniverseConstness<UniverseT> *getArbitraryFrameImpl(UniverseT &universe, int frameNumber);
    template<typename UniverseT>
    static FrameMatchingUniverseConstness<UniverseT> *getArbitraryFrameClampedImpl(UniverseT &universe, int frameNumber);

    friend Frame *getEntryFrame(Universe &universe, TimeDirection direction);
    friend Frame const *getEntryFrame(Universe const &universe, TimeDirection direction);
    
    friend Frame *getArbitraryFrame(Universe &universe, int frameNumber);
    friend Frame const *getArbitraryFrame(Universe const &universe, int frameNumber);
    
    friend Frame *getArbitraryFrameClamped(Universe &universe, int frameNumber);
    friend Frame const *getArbitraryFrameClamped(Universe const &universe, int frameNumber);
    
    friend int getTimelineLength(Universe const &universe);

    boost::container::vector<Frame> frames;
};

inline void swap(Universe &l, Universe &r)
{
    Universe temp(std::move(l));
    l = std::move(r);
    r = std::move(temp);
}
}
#endif //HG_UNIVERSE_H
