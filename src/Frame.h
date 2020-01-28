#ifndef HG_FRAME_H
#define HG_FRAME_H

#include "TimeDirection.h"
#include "ObjectList.h"
#include "ObjectPtrList.h"
#include "ObjectListTypes.h"
#include "FrameView.h"
#include <boost/fusion/container/vector.hpp>
#include <boost/range/adaptor/map.hpp>

#include <tbb/concurrent_hash_map.h>
#include "mt/boost/container/map.hpp"
#include "mt/std/vector"
#include "mt/std/map"

#include "Universe_fwd.h"
#include "FrameUpdateSet_fwd.h"
#include "FrameID_fwd.h"

#include "memory_pool.h"

namespace hg {
struct Universe_Frame_access final {
    friend class Universe;
private:
    Universe_Frame_access(){}
};

bool isNullFrame(Frame const *frame);
Frame const *nextFrame(Frame const *frame, TimeDirection direction);
Frame *nextFrame(Frame *frame, TimeDirection direction);
bool nextFrameInSameUniverse(Frame const *frame, TimeDirection direction);
Universe &getUniverse(Frame *frame);
Universe const &getUniverse(Frame const *frame);
int getFrameNumber(Frame const *frame);
unsigned getFrameSpeedOfTime(Frame const *frame);
//Only one "Frame" per frame. Referenced by frame pointers and contained in universes.
class Frame final {
public:
    typedef mt::std::map<Frame *, ObjectList<Normal>> FrameDeparturesT;
    typedef tbb::concurrent_hash_map<Frame const *, ObjectList<Normal> const *> FrameArrivalsT;
    Frame(int frameNumber, unsigned frameSpeedOfTime, Universe &universe);
    #if 0
    Frame(Frame const &o) :
        frameNumber(o.frameNumber),
        universe(o.universe),
        //TODO: Consider making initial size match old departure pool size
        departurePoolA(),
        departurePoolB(),
        //TODO: Move departures to use new departurePool
        departures(o.departures),
        arrivals(o.arrivals),
        view(o.view)
    {}
    #endif
    //returns the frames whose arrivals are changed
    //newDeparture may get its contents pilfered
    FrameUpdateSet updateDeparturesFromHere(FrameDeparturesT &&newDeparture);

    void setSpeedOfTime(int newSpeedOfTime) { frameSpeedOfTime = newSpeedOfTime; }

    void setView(FrameView &&newView) { view.swap(newView); }
    FrameView const &getView() const { return view; }
    /**
     * Returns a flattened view of the arrivals to 'time' for passing to the physics engine.
     */
    ObjectPtrList<Normal> getPrePhysics() const;

    /**
     * Returns a flattened view of the departures from 'time' for passing to the front-end.
     * These departures are filtered to be only the departures departing to `toUniverse'.
     * "Departing to `toUniverse'" is defined as "departing to a frame that is either in `toUniverse',
     * or the nullFrame". "in `toUniverse'" does not include frames in a subUniverse of `toUniverse'.
     */
    ObjectPtrList<Normal> getPostPhysics() const;

    //Used to initially set the pointer to the permanent arrival, and to change the pointers
    //when the TimeEngine is copied. Note that if the newPermanentArrival is not equivalent
    //to the permanent arrival already held by the frame, the frame must be re-executed (by WorldState).
    void setPermanentArrival(ObjectList<Normal> const *newPermanentArrival);
private:
public: //Pseudo-private, only accessible from Universe:
    //These "correct" functions are for rearranging pointers when universes get copied.
    void correctUniverse(Universe_Frame_access, Universe &newUniverse) noexcept;
    void correctDepartureFramePointers(Universe_Frame_access, FramePointerUpdater const &updater);
    void correctArrivalObjectListPointers(Universe_Frame_access);
    void correctArrivalFramePointers(Universe_Frame_access, FramePointerUpdater const &updater);
private:
    //Private to enforce use of non-member variants.
    Frame const *nextFrame(TimeDirection direction) const;
    Frame *nextFrame(TimeDirection direction);
    bool nextFrameInSameUniverse(TimeDirection direction) const;
    Universe const &getUniverse() const;
    Universe &getUniverse();
    int getFrameNumber() const;
    unsigned getFrameSpeedOfTime() const;
    //Valid to call with NullFrame {
    friend bool isNullFrame(Frame const *frame);
    //}
    //Undefined to call with NullFrame {
    friend Frame const *nextFrame(Frame const *frame, TimeDirection direction);
    friend Frame *nextFrame(Frame *frame, TimeDirection direction);
    friend bool nextFrameInSameUniverse(Frame const *frame, TimeDirection direction);
    friend Universe &getUniverse(Frame *frame);
    friend Universe const &getUniverse(Frame const *frame);
    friend int getFrameNumber(Frame const *frame);
    friend unsigned getFrameSpeedOfTime(Frame const *frame);
    //}
    void insertArrival(FrameArrivalsT::value_type const &toInsert);
    void changeArrival(FrameArrivalsT::value_type const &toChange);
    void clearArrival(Frame const *toClear);

    // Position of frame within universe_
    int frameNumber;
    // Speed of time on frame
    unsigned frameSpeedOfTime;
    // Back-link to universe which this frame is in
    Universe *universe;
    #if 0
#if USE_POOL_ALLOCATOR
    //TODO: allow memory_pool to be copied
    memory_pool<user_allocator_tbb_alloc> departurePoolA;
    memory_pool<user_allocator_tbb_alloc> departurePoolB;
#else
    memory_pool<> departurePoolA;
    memory_pool<> departurePoolB;
#endif
    #endif
    //Arrival departure map stuff. Could instead be put in external hash-map keyed by Frame*
    FrameDeparturesT departures;
    FrameArrivalsT arrivals;

        //const std::size_t initialPoolSize{2<<5};
    //memory_pool<user_allocator_tbb_alloc> pool{initialPoolSize};


    FrameView view;
};
}
#endif //HG_FRAME_H
