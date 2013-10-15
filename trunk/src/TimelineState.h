#ifndef HG_ARRIVAL_DEPARTURE_MAP_H
#define HG_ARRIVAL_DEPARTURE_MAP_H


#include "Universe.h"
#include "ObjectList.h"
#include "ObjectPtrList.h"
#include "ObjectListTypes.h"

#include <tbb/task.h>

#include <boost/container/map.hpp>
#include <boost/move/move.hpp>

#include "FrameID_fwd.h"
#include "Frame_fwd.h"
#include "FrameUpdateSet_fwd.h"
#include "DepartureMap_fwd.h"

namespace hg {
class TimelineState
{
public:
    /**
     * Constructs a timeline state of length timeLength containing no arrivals or departures.
     */
    explicit TimelineState(std::size_t timelineLength);
    
    TimelineState(BOOST_RV_REF(TimelineState) o) :
        universe_(boost::move(o.universe_)),
        permanentDepartures_(boost::move(o.permanentDepartures_))
    {
    }
    TimelineState &operator=(BOOST_RV_REF(TimelineState) o)
    {
        universe_ = boost::move(o.universe_);
        permanentDepartures_ = boost::move(o.permanentDepartures_);
        return *this;
    }

    void swap(TimelineState &o);
    
    /**
     * Updates the timeline with new departures and returns the set of frames
     * whose arrivals have changed.
     */
    FrameUpdateSet updateWithNewDepartures(
    	DepartureMap &newDepartures/*, tbb::task_group_context &context*/);

    /**
     * Creates the arrivals for those objects initially in the level.
     * initialArrivals must contain those arrivals.
     * This should only be called once.
     */
    //Consider moving this into constructor
    void addArrivalsFromPermanentDepartureFrame(
    	std::map<Frame *, ObjectList<Normal> > const &initialArrivals);

    /**
     * Converts FrameID into Frame*
     */
    Frame *getFrame(FrameID const &whichFrame);
    Universe &getUniverse() {
        return universe_;
    }
    
    Universe const &getUniverse() const {
        return universe_;
    }
    
private:
    Universe universe_;
    boost::container::map<Frame*, ObjectList<Normal> > permanentDepartures_;
    BOOST_MOVABLE_BUT_NOT_COPYABLE(TimelineState)
};
inline void swap(TimelineState &l, TimelineState &r) { l.swap(r); }
}
#endif //HG_ARRIVAL_DEPARTURE_MAP_H
