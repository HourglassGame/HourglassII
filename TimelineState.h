#ifndef HG_ARRIVAL_DEPARTURE_MAP_H
#define HG_ARRIVAL_DEPARTURE_MAP_H

#include "TimeObjectListList.h"
#include "FrameID.h"
#include <boost/unordered_map.hpp>

namespace hg {
class FrameUpdateSet;
class DepartureMap;
class TimelineState
{
public:
    /**************
     * Constructs a time line state of length timeLength containing no arrivals or departures.
     */
	TimelineState();

    /********************
     * Updates the timeline with new departures and returns the set of frames
     * whose arrivals have changed.
     */
    FrameUpdateSet updateWithNewDepartures(const DepartureMap& newDepartures);

    /******************
     * Creates the arrivals for those objects initially in the level.
     * initialArrivals must contain those arrivals.
     * This should only be called once.
     */
     //Consider moving this into constructor
    void addArrivalsFromPermanentDepartureFrame(const TimeObjectListList& initialArrivals);

    /*****************************************************
     * Returns a flattened view of the arrivals to 'time' for passing to the physics engine.
     */
	ObjectList getPrePhysics(const FrameID& time) const;

    /*****************************************************
     * Returns a flattened view of the departures from 'time' for passing to the front-end.
     */
    ObjectList getPostPhysics(const FrameID& time, const PauseInitiatorID& whichPrePause) const;

    /**********************
     * A convenience class to represent a single frame of the timeline.
     */
    class Frame {
    public:

    /*****************************************************
     * Returns a flattened view of the arrivals to this frame for passing to the physics engine.
     */
        ObjectList getPrePhysics() const;

    /*****************************************************
     * Returns the ID for the frame that this Frame represents.
     */
        FrameID getTime() const;
    private:
        friend class TimelineState;
        Frame(const TimelineState& mapPtr, const FrameID& time);
        FrameID time_;
        const TimelineState& this_;
    };

    /***************************************
     * Returns a Frame for whichFrame.
     */
    Frame getFrame(const FrameID& whichFrame) const;
private:
	//SimpleFrameID permanentDepartureIndex;
    ::boost::unordered_map<FrameID, TimeObjectListList> arrivals;
    ::boost::unordered_map<FrameID, TimeObjectListList> departures;

    FrameUpdateSet updateDeparturesFromTime(const FrameID& time, const TimeObjectListList& newDeparture);
};
}
#endif //HG_ARRIVAL_DEPARTURE_MAP_H
