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
class FrameUpdateSet;
class Frame;
class FrameID;
class FramePtr;
class Universe;
//Wrapper around Frame* to give extra safety (assert ptr before dereferencing,
//    also allows shared syntax with FrameID for ease of changing).
/*
class FramePtr {
public:
    //NullFrame construction
    FramePtr() :
    framePtr_(0)
    {
    }
    //assignment
    FramePtr& operator=(const FramePtr& other)
    {
        framePtr_ = other.framePtr_;
    }
    
    //copy-construction
    FramePtr(const FramePtr& other) :
    framePtr_(other.framePtr_)
    {
    }
    
    FramePtr(Frame* ptr) :
    framePtr_(ptr)
    {
    }
    
    // returns the normal next frame for things moving in direction TimeDirection
    FramePtr nextFrame(TimeDirection direction) const
    {
        assert(framePtr_);
        return framePtr_->nextFrame(direction);
    }

    // returns if the next frame for things moving in direction TimeDirection
    //is part of the same pause time universe as the frame
    bool nextFramePauseLevelDifference(TimeDirection direction) const
    {
        assert(framePtr_);
        return framePtr_->nextFramePauseLevelDifference(direction);
    }

    // returns a frameID using frameNumber as 'distance' from the start of the universe in
    FramePtr arbitraryFrameInUniverse(unsigned int frameNumber) const
    {
        assert(framePtr_);
        return framePtr_->arbitraryFrameInUniverse(frameNumber);
    }

    // returns the frame that spawned the universe that this frame is in
    FramePtr parentFrame() const
    {
        assert(framePtr_);
        return framePtr_->parentFrame();
    }

    // returns frameID of child frame in the universe defined by the first 2 arguments with frameNumber as
    //'distance' from the start of the universe This function cannot return nullFrame,
    //place assert to assure frameNumber is never greater pauseLength
    FramePtr arbitraryChildFrame(const PauseInitiatorID& initatorID, unsigned int frameNumber) const
    {
        assert(framePtr_);
        return framePtr_->arbitraryChildFrame(initatorID,direction);
    }

    // returns the frameID of child frame at beginning or end of universe defined by first 2 arguments,
    //FORWARDS returns arbitaryChildFrame frameNumber 0 and REVERSE returns with the last frame of the
    //universe cannot return nullFrame,
    FramePtr entryChildFrame(const PauseInitiatorID& initatorID, TimeDirection direction) const
    {
        assert(framePtr_);
        return framePtr_->entryChildFrame(initatorID,direction);
    }

    bool operator==(const FramePtr other)
    {
        assert(framePtr_);
        return framePtr_ == other.framePtr_;
    }

    bool operator<(const FramePtr other) 
    {
        return framePtr_ < other.framePtr_;
    }
    
    operator Frame*()
    {
        return framePtr_;
    }
private:
    friend ::std::size_t hash_value(const FramePtr& toHash);
    friend class Universe;

    //constructs a FramePtr for the frame at time in the universe given by Universe
    FramePtr(unsigned int time, const Universe& universe);

    Frame* framePtr_;
};*/
//Only one frame per frame. Referenced by frame pointers and contained in universes.
//Tim's idea for FrameID
//A system like this could also put the arrivals and departures in Frames
//and so avoid the arrival-departure-map system altogether
class Frame {
//    #if 0
    typedef ::boost::unordered_map<PauseInitiatorID, Universe> SubUniverseMap;
    typedef ::tbb::concurrent_hash_map<Frame*, ObjectList*> ArrivalMap;
    public:
    Frame(unsigned int frameNumber, Universe* universe);
    
    //copy-construction -- quite dangerous, because frames are identified by their address
    Frame(const Frame& other);
    
    // returns the normal next frame for things moving in direction TimeDirection
    Frame* nextFrame(TimeDirection direction) const;

    // returns if the next frame for things moving in direction TimeDirection
    //is part of the same pause time universe as the frame
    unsigned int nextFramePauseLevelDifference(TimeDirection direction) const;

    // returns a frameID using frameNumber as 'distance' from the start of the universe in
    Frame* arbitraryFrameInUniverse(unsigned int frameNumber) const;

    // returns the frame that spawned the universe that this frame is in
    Frame* parentFrame() const;

    // returns frameID of child frame in the universe defined by the first 2 arguments with frameNumber as
    //'distance' from the start of the universe This function cannot return nullFrame,
    //place assert to assure frameNumber is never greater than pauseLength - 1
    Frame* arbitraryChildFrame(const PauseInitiatorID& initiatorID, unsigned int frameNumber);

    // returns the frameID of child frame at beginning or end of universe defined by first 2 arguments,
    //FORWARDS returns arbitaryChildFrame frameNumber 0 and REVERSE returns with the last frame of the
    //universe cannot return nullFrame,
    Frame* entryChildFrame(const PauseInitiatorID& initiatorID, TimeDirection direction);
    //returns the frames whose arrivals are changed
    //newDeparture may get its contents pilfered
    FrameUpdateSet updateDeparturesFromHere(std::map<Frame*, ObjectList>& newDeparture);
    
    //assignment
    Frame& operator=(const Frame& other)
    {
        //Should never be called. Needed for vector.push_back, but I have already reserved space.
        assert(false);
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
    //FrameID toFrameID() const;
    unsigned int getFrameNumber() const { return frameNumber_; }
    const PauseInitiatorID& getInitiatorID() const;
    private:
    unsigned int nextFramePauseLevelDifferenceAux(TimeDirection direction, int accumulator) const;
    
    void insertArrival(const ArrivalMap::value_type& toInsert);
    void changeArrival(const ArrivalMap::value_type& toChange);
    void clearArrival(Frame* toClear);
    unsigned int frameNumber_;
    //back-link to universe in which this frame is
    Universe* universe_;
    std::map<Frame*, ObjectList> departures_;
    ArrivalMap arrivals_;
    SubUniverseMap subUniverses_;
 //   #endif
};
}
#endif //HG_FRAME_H
