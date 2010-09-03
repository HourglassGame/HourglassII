#ifndef HG_ARRIVAL_DEPARTURE_MAP_H
#define HG_ARRIVAL_DEPARTURE_MAP_H

#include "TimeObjectListList.h"
#include "FrameID.h"
#include <vector>
#include <map>

namespace hg {
class TimelineState
{
public:
	TimelineState(unsigned int timeLength);
    
    ::std::vector<unsigned int> updateDeparturesFromTime(unsigned int time, const TimeObjectListList& newDeparture);

	ObjectList& permanentDepartureObjectList(unsigned int arrivalTime);

	ObjectList getPrePhysics(FrameID time) const;
    ObjectList getPostPhysics(FrameID time) const;
    
    ::std::vector<FrameID> updateWithNewDepartures(const ::std::map<FrameID, TimeObjectListList>& newDepartures);
    
    bool operator==(const TimelineState& other) const;
    inline bool operator!=(const TimelineState& other) const {
        return !(*this==other);
    }
    
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
	unsigned int permanentDepartureIndex;
    ::std::vector<TimeObjectListList> arrivals;
    ::std::vector<TimeObjectListList> departures;
};
}
#endif //HG_ARRIVAL_DEPARTURE_MAP_H
