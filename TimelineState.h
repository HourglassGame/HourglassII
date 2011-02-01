#ifndef HG_ARRIVAL_DEPARTURE_MAP_H
#define HG_ARRIVAL_DEPARTURE_MAP_H

#include <map>
#include "Universe.h"

namespace hg {
class FrameID;
class Frame;
class FrameUpdateSet;
class DepartureMap;
class TimelineState
{
public:
    /**************
     * Constructs a timeline state of length timeLength containing no arrivals or departures.
     */
	TimelineState(unsigned int timelineLength);

    /********************
     * Updates the timeline with new departures and returns the set of frames
     * whose arrivals have changed.
     */
    FrameUpdateSet updateWithNewDepartures(DepartureMap& newDepartures);

    /******************
     * Creates the arrivals for those objects initially in the level.
     * initialArrivals must contain those arrivals.
     * This should only be called once.
     */
     //Consider moving this into constructor
    void addArrivalsFromPermanentDepartureFrame(std::map<Frame*, ObjectList>& initialArrivals);

    /***************************************
     * Converts FrameID into Frame*
     */
    Frame* getFrame(const FrameID& whichFrame);
    Universe& getUniverse() {return universe_;}
private:
    Universe universe_;
    std::map<Frame*, ObjectList> permanentDepartures_;
};
}
#endif //HG_ARRIVAL_DEPARTURE_MAP_H
