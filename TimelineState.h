#ifndef HG_ARRIVAL_DEPARTURE_MAP_H
#define HG_ARRIVAL_DEPARTURE_MAP_H


#include "Universe.h"
#include "ObjectList.h"
#include "ObjectPtrList.h"
#include "ObjectListTypes.h"

#include <map>

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
    
    void swap(TimelineState& other);
    
    /**
     * Updates the timeline with new departures and returns the set of frames
     * whose arrivals have changed.
     */
    FrameUpdateSet updateWithNewDepartures(DepartureMap& newDepartures);

    /**
     * Creates the arrivals for those objects initially in the level.
     * initialArrivals must contain those arrivals.
     * This should only be called once.
     */
    //Consider moving this into constructor
    void addArrivalsFromPermanentDepartureFrame(std::map<Frame*, ObjectList<Normal> > const& initialArrivals);

    /**
     * Converts FrameID into Frame*
     */
    Frame* getFrame(FrameID const& whichFrame);
    Universe& getUniverse() {
        return universe_;
    }
private:
    Universe universe_;
    std::map<Frame*, ObjectList<Normal> > permanentDepartures_;
    //intentionally undefined
    TimelineState(TimelineState const& other);
    TimelineState& operator=(TimelineState const& other);
};
inline void swap(TimelineState& l, TimelineState& r) { l.swap(r); }
}
#endif //HG_ARRIVAL_DEPARTURE_MAP_H
