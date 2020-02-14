#ifndef HG_ARRIVAL_DEPARTURE_MAP_H
#define HG_ARRIVAL_DEPARTURE_MAP_H


#include "Universe.h"
#include "ArrivalDepartures/ObjectList.h"
#include "ArrivalDepartures/ObjectPtrList.h"
#include "ArrivalDepartures/ObjectListTypes.h"
#include "ConcurrentFrameUpdateSet.h"

#include <tbb/task.h>

#include <boost/container/map.hpp>
#include <utility>

#include "FrameID_fwd.h"
#include "Frame_fwd.h"
#include "FrameUpdateSet_fwd.h"
#include "DepartureMap_fwd.h"

namespace hg {

class TimelineState final
{
    static boost::container::map<Frame*, ObjectList<Normal>> fixPermanentDepartures(
        FramePointerUpdater const& framePointerUpdater,
        boost::container::map<Frame*, ObjectList<Normal>> const& oldPermanentDepartures);
public:
    /**
     * Constructs a timeline state of length timeLength containing no arrivals or departures.
     */
    explicit TimelineState(std::size_t timelineLength, unsigned defaultSpeedOfTime);
    
    TimelineState(TimelineState const& o) :
        universe_(o.universe_),
        permanentDepartures_(fixPermanentDepartures(FramePointerUpdater(universe_), o.permanentDepartures_))
    {
    }
    TimelineState &operator=(TimelineState const& o)
    {
        universe_ = o.universe_;
        permanentDepartures_ = fixPermanentDepartures(FramePointerUpdater(universe_), o.permanentDepartures_);
        return *this;
    }
    
    TimelineState(TimelineState&& o) noexcept :
        universe_(std::move(o.universe_)),
        permanentDepartures_(std::move(o.permanentDepartures_))
    {}
    TimelineState &operator=(TimelineState&& o) noexcept {
        universe_ = std::move(o.universe_);
        permanentDepartures_ = std::move(o.permanentDepartures_);
        return *this;
    }

    void swap(TimelineState &o) noexcept;
    
    /**
     * Updates the timeline with new departures and returns the set of frames
     * whose arrivals have changed.
     */
    FrameUpdateSet updateWithNewDepartures(
        DepartureMap &newDepartures,
        ConcurrentFrameUpdateSet &framesWithChangedArrivals /*, tbb::task_group_context &context */);

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
    Frame       *getFrame(FrameID const &whichFrame) {
        return universe_.getFrame(whichFrame);
    }
    Frame const *getFrame(FrameID const &whichFrame) const {
        return universe_.getFrame(whichFrame);
    }
    
    Universe       &getUniverse()       { return universe_; }
    Universe const &getUniverse() const { return universe_; }
    
private:
    Universe universe_;
    boost::container::map<Frame*, ObjectList<Normal>> permanentDepartures_;
};
inline void swap(TimelineState &l, TimelineState &r) noexcept { l.swap(r); }
}
#endif //HG_ARRIVAL_DEPARTURE_MAP_H
