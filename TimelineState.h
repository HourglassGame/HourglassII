#ifndef HG_ARRIVAL_DEPARTURE_MAP_H
#define HG_ARRIVAL_DEPARTURE_MAP_H

#include "TimeObjectListList.h"
#include "FrameID.h"
#include <vector>

namespace hg {
class FrameUpdateSet;
class DepartureMap;
class TimelineState
{
public:
	TimelineState(unsigned int timeLength);
    
    FrameUpdateSet updateDeparturesFromTime(FrameID time, const TimeObjectListList& newDeparture);

    void setArrivalsFromPermanentDepartureFrame(TimeObjectListList& initialArrivals);

	ObjectList getPrePhysics(FrameID time) const;
    ObjectList getPostPhysics(FrameID time) const;
    
    FrameUpdateSet updateWithNewDepartures(const DepartureMap& newDepartures);
    
    class Frame {
    public:
        ObjectList getPrePhysics() const;
        FrameID getTime() const;
    private:
        friend class TimelineState;
        Frame(const TimelineState& mapPtr, FrameID time);
        FrameID time_;
        const TimelineState& this_;
    };
    Frame getFrame(FrameID whichFrame) const;
private:
	FrameID permanentDepartureIndex;
    ::std::vector<TimeObjectListList> arrivals;
    ::std::vector<TimeObjectListList> departures;
};
}
#endif //HG_ARRIVAL_DEPARTURE_MAP_H
