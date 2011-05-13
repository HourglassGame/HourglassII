#ifndef HG_FRAME_H
#define HG_FRAME_H

#include "TimeDirection.h"
#include "PauseInitiatorID.h"
#include "ObjectList.h"
//LOLOLOL all 3 types of map needed (:
#include <tbb/concurrent_hash_map.h>
#include <boost/unordered_map.hpp>
#include <map>

#include "Universe_fwd.h"
#include "FrameUpdateSet_fwd.h"
#include "FrameID_fwd.h"
namespace hg {
//Only one "Frame" per frame. Referenced by frame pointers and contained in universes.
//Arrivals and departures are also held by
class Frame {
    typedef boost::unordered_map<PauseInitiatorID, Universe> SubUniverseMap;
    typedef tbb::concurrent_hash_map<Frame*, ObjectList*> ArrivalMap;
public:
    //copy-construction -- quite dangerous, because frames are identified by their address
    //public to allow use in standard containers in Universe
    Frame(const Frame& other);

    Frame(std::size_t frameNumber, Universe& universe);
    
    //returns the frames whose arrivals are changed
    //newDeparture may get its contents pilfered
    FrameUpdateSet updateDeparturesFromHere(std::map<Frame*, ObjectList>& newDeparture);

    //assignment
    Frame& operator=(const Frame&)
    {
        //Should never be called. Needed for vector<Frame>::push_back to compile
        //(in Universe construction), but I have already reserved space.
        assert(false);
        return *this;
    }
    /**
     * Returns a flattened view of the arrivals to 'time' for passing to the physics engine.
     */
    ObjectPtrList getPrePhysics() const;

    /**
     * Returns a flattened view of the departures from 'time' for passing to the front-end.
     * Not sure what the whichPrePause argument is actually there for, so leaving that functionality out for now.
     */
    ObjectPtrList getPostPhysics(/*const PauseInitiatorID& whichPrePause*/) const;
    void addArrival(Frame* source, ObjectList* arrival);


private:
    friend class FrameID;
    friend class Universe;
    friend class UniverseID;

    //Private to enforce use of non-member variants.
    Frame* nextFrame(TimeDirection direction) const;
    unsigned int nextFramePauseLevelDifference(TimeDirection direction) const;
    bool nextFrameInSameUniverse(TimeDirection direction) const;
    Universe& getSubUniverse(PauseInitiatorID const& initiatorID);
    Universe& getUniverse() const;
    std::size_t getFrameNumber() const;
    
    friend Frame* nextFrame(Frame const* frame, TimeDirection direction);
    friend bool nextFrameInSameUniverse(Frame const* frame, TimeDirection direction);
    friend unsigned int nextFramePauseLevelDifference(Frame const* frame, TimeDirection direction);
    friend Universe& getUniverse(Frame const* frame);
    friend Universe& getSubUniverse(Frame* frame, PauseInitiatorID const& initiatorID);
    friend std::size_t getFrameNumber(Frame const* frame);
    unsigned int nextFramePauseLevelDifferenceAux(TimeDirection direction, unsigned int accumulator) const;

    void insertArrival(const ArrivalMap::value_type& toInsert);
    void changeArrival(const ArrivalMap::value_type& toChange);
    void clearArrival(Frame* toClear);
    /** Position of frame within universe_ */
    std::size_t frameNumber_;
    /** Back-link to universe which this frame is in */
    Universe& universe_;
    std::map<Frame*, ObjectList> departures_;
    ArrivalMap arrivals_;
    SubUniverseMap subUniverses_;
};
//<Undefined to call with NullFrame>
//Frame* nextFrame(Frame const* frame, TimeDirection direction);
//bool nextFrameInSameUniverse(Frame const* frame, TimeDirection direction);
//unsigned int nextFramePauseLevelDifference(Frame const* frame, TimeDirection direction);
//Universe& getUniverse(Frame const* frame);
//Universe& getSubUniverse(Frame* frame, PauseInitiatorID const& initiatorID);
//std::size_t getFrameNumber(Frame const* frame);
//</Undefined to call with NullFrame>
//<Valid to call with NullFrame>
bool isNullFrame(Frame const* frame);
//</Valid to call with NullFrame>
}
#endif //HG_FRAME_H
