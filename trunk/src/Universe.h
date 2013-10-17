#ifndef HG_UNIVERSE_H
#define HG_UNIVERSE_H

#include "Frame.h"
#include "FrameID_fwd.h"
#include "TimeDirection.h"
#include <boost/container/vector.hpp>
#include <boost/move/move.hpp>
#include "Universe_fwd.h"
namespace hg {
class Universe {
public:
    //creates a top level universe
    explicit Universe(int timelineLength);

    Universe(BOOST_RV_REF(Universe) o);
    Universe &operator=(BOOST_RV_REF(Universe) o);

    //Conversion from FrameID to equivalent Frame * within this universe
    //whichFrame must correspond to a frame that could actually
    //be in the universe.
    Frame *getFrame(FrameID const &whichFrame);

private:
    friend class Frame;
    friend class UniverseID;
    void fixFramesUniverses();

    //<UniverseT interface>
    //Returns the first frame in the universe for objects traveling
    //in TimeDirection direction.
    Frame *getEntryFrame(TimeDirection direction);
    //Returns the frame with the index frameNumber within the universe,
    //or the NullFrame if no such frame exists
    Frame *getArbitraryFrame(int frameNumber);
    //Returns the frame with the index closest to frameNumber within the universe.
    Frame *getArbitraryFrameClamped(int frameNumber);
    //returns the length of this Universe's timeline
    int getTimelineLength() const;
    
    //</UniverseT interface>
    //Returns the first frame in the universe for objects traveling
    //in TimeDirection direction.
    friend Frame *getEntryFrame(Universe &universe, TimeDirection direction);
    //Returns the frame with the index frameNumber within the universe,
    //or the NullFrame if no such frame exists
    friend Frame *getArbitraryFrame(Universe &universe, int frameNumber);
    //Returns the frame with the index closest to frameNumber within the universe.
    friend Frame *getArbitraryFrameClamped(Universe &universe, int frameNumber);
    //returns the length of this universe's timeline
    friend int getTimelineLength(Universe const &universe);

    boost::container::vector<Frame> frames_;

    BOOST_MOVABLE_BUT_NOT_COPYABLE(Universe)
};
/*
Frame *getEntryFrame(Universe &universe, TimeDirection direction);
Frame *getArbitraryFrame(Universe &universe, int frameNumber);
 */
int getTimelineLength(Universe const &universe);

inline void swap(Universe &l, Universe &r)
{
    Universe temp(boost::move(l));
    l = boost::move(r);
    r = boost::move(temp);
}
}
#endif //HG_UNIVERSE_H
