#ifndef HG_FRAME_H
#define HG_FRAME_H

#include "TimeDirection.h"
#include "PauseInitiatorID.h"
#include "ObjectList.h"
#include "Universe.h"
//LOLOLOL all 3 types of map needed (:
#include <tbb/concurrent_hash_map.h>
#include <boost/unordered_map.hpp>
#include <map>
namespace hg {
class Universe;
class FrameUpdateSet;
class FrameID;
//Only one frame per frame. Referenced by frame pointers and contained in universes.
//Tim's idea for FrameID
//A system like this could also put the arrivals and departures in Frames
//and so avoid the arrival-departure-map system altogether
class Frame {
    typedef boost::unordered_map<PauseInitiatorID, Universe> SubUniverseMap;
    typedef tbb::concurrent_hash_map<Frame*, ObjectList*> ArrivalMap;
    public:
    //copy-construction -- quite dangerous, because frames are identified by their address
    //public to allow use in standard containers in Universe
    Frame(const Frame& other);
    
    Frame(std::size_t frameNumber, Universe& universe);
    
    // returns the normal next frame for things moving in direction TimeDirection
    Frame* nextFrame(TimeDirection direction) const;

    // returns if the next frame for things moving in direction TimeDirection
    //is part of the same pause time universe as the frame
    unsigned int nextFramePauseLevelDifference(TimeDirection direction) const;

    // returns a frameID using frameNumber as 'distance' from the start of the universe in
    Frame* arbitraryFrameInUniverse(std::size_t frameNumber) const;

    // returns the frame that spawned the universe that this frame is in
    Frame* parentFrame() const;

    // returns frameID of child frame in the universe defined by the first 2 arguments with frameNumber as
    //'distance' from the start of the universe This function cannot return nullFrame,
    //place assert to assure frameNumber is never greater than pauseLength - 1
    Frame* arbitraryChildFrame(const PauseInitiatorID& initiatorID, std::size_t frameNumber);

    // returns the frameID of child frame at beginning or end of universe defined by first 2 arguments,
    //FORWARDS returns arbitaryChildFrame frameNumber 0 and REVERSE returns with the last frame of the
    //universe cannot return nullFrame,
    Frame* entryChildFrame(const PauseInitiatorID& initiatorID, TimeDirection direction);
    //returns the frames whose arrivals are changed
    //newDeparture may get its contents pilfered
    FrameUpdateSet updateDeparturesFromHere(std::map<Frame*, ObjectList>& newDeparture);
    
    //assignment
    Frame& operator=(const Frame&)
    {
        //Should never be called. Needed for vector<Frame>::push_back  (in Universe construction), but I have already reserved space.
        assert(false);
        return *this;
    }
    /*****************************************************
     * Returns a flattened view of the arrivals to 'time' for passing to the physics engine.
     */
	ObjectPtrList getPrePhysics() const;

    /*****************************************************
     * Returns a flattened view of the departures from 'time' for passing to the front-end.
     * Not sure what the whichPrePause argument is actually there for, so leaving that functionality out for now.
     */
    ObjectPtrList getPostPhysics(/*const PauseInitiatorID& whichPrePause*/) const;
    void addArrival(Frame* source, ObjectList* arrival);
    std::size_t getFrameNumber() const { return frameNumber_; }
    const PauseInitiatorID& getInitiatorID() const;
    private:
    friend class FrameID;
    friend class Universe;
    friend class UniverseID;

    unsigned int nextFramePauseLevelDifferenceAux(TimeDirection direction, unsigned int accumulator) const;
    
    void insertArrival(const ArrivalMap::value_type& toInsert);
    void changeArrival(const ArrivalMap::value_type& toChange);
    void clearArrival(Frame* toClear);
    //position of frame within universe_
    std::size_t frameNumber_;
    //back-link to universe which this frame is in
    Universe& universe_;
    std::map<Frame*, ObjectList> departures_;
    ArrivalMap arrivals_;
    SubUniverseMap subUniverses_;
};
}
#endif //HG_FRAME_H
