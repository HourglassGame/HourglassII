#ifndef HG_FRAME_H
#define HG_FRAME_H

#include "TimeDirection.h"
#include "ObjectList.h"
#include "ObjectPtrList.h"
#include "ObjectListTypes.h"

#include <boost/fusion/container/vector.hpp>
#include <boost/range/adaptor/map.hpp>

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
public:
    //copy-construction -- quite dangerous, because frames are identified by their address
    //public to allow use in standard containers in Universe
    Frame(const Frame& other);

    Frame(std::size_t frameNumber, Universe& universe);

    //returns the frames whose arrivals are changed
    //newDeparture may get its contents pilfered    
    FrameUpdateSet updateDeparturesFromHere(std::map<Frame*, ObjectList<Normal> >& newDeparture);
    
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
    ObjectPtrList<Normal>  getPrePhysics() const;

    /**
     * Returns a flattened view of the departures from 'time' for passing to the front-end.
     * These departures are filtered to be only the departures departing to `toUniverse'.
     * "Departing to `toUniverse'" is defined as "departing to a frame that is either in `toUniverse',
     * or the nullFrame". "in `toUniverse'" does not include frames in a subUniverse of `toUniverse'.
     */
    ObjectPtrList<Normal>  getPostPhysics() const;

    //Used for adding arrivals from permanent departure frame
    void addArrival(Frame const* source, ObjectList<Normal> const* arrival);

private:
    friend class FrameID;
    friend class Universe;
    friend class UniverseID;

    //Private to enforce use of non-member variants.
    Frame const* nextFrame(TimeDirection direction) const;
    Frame* nextFrame(TimeDirection direction);
    bool nextFrameInSameUniverse(TimeDirection direction) const;
    Universe const& getUniverse() const;
    Universe& getUniverse();
    std::size_t getFrameNumber() const;
    
    friend Frame const* nextFrame(Frame const* frame, TimeDirection direction);
    friend Frame* nextFrame(Frame* frame, TimeDirection direction);
    friend bool nextFrameInSameUniverse(Frame const* frame, TimeDirection direction);
    friend Universe& getUniverse(Frame * frame);
    friend Universe const& getUniverse(Frame const* frame);
    friend std::size_t getFrameNumber(Frame const* frame);

    //Temporal comparison. Frames compare equal to themselves and all their parents or children.
    //Frames compare greater than or less than all other frames.
    //If l and r are in the same universe then l < r (direction) iff r can be obtained from l by calls to l = getNextFrame(l, direction)
    //If l and r are in different universes then l < r (direction) iff 
    //This comparison does not establish a total order.
    //If l or r is the null frame then behaviour is undefined.
    //You cannot rely on such things as a == b && b == c => a == c
    //usage: a <op> b => compare(a, b) <op> 0
    friend int compare(Frame const* l, Frame const* r, TimeDirection direction);

    void insertArrival(const tbb::concurrent_hash_map<Frame const*, ObjectList<Normal> const*>::value_type& toInsert);
    void changeArrival(const tbb::concurrent_hash_map<Frame const*, ObjectList<Normal> const*>::value_type& toChange);
    void clearArrival(Frame const* toClear);
    
    /** Position of frame within universe_ */
    std::size_t frameNumber_;
    /** Back-link to universe which this frame is in */
    Universe& universe_;
    
    //Arrival departure map stuff. Could instead be put in external hash-map keyed by Frame*
    std::map<Frame*, ObjectList<Normal> > departures_;
    tbb::concurrent_hash_map<Frame const*, ObjectList<Normal> const*> arrivals_;
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
