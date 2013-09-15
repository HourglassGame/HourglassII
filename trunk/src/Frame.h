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

#include "Universe_fwd.h"
#include "FrameUpdateSet_fwd.h"
#include "FrameID_fwd.h"
namespace hg {
//Only one "Frame" per frame. Referenced by frame pointers and contained in universes.
class Frame {
public:
    typedef mt::boost::container::map<Frame *, ObjectList<Normal> >::type FrameDeparturesT;

    Frame(int frameNumber, Universe& universe);

    Frame(Frame const& o) :
        frameNumber_(o.frameNumber_),
        universe_(o.universe_),
        departures_(o.departures_),
        arrivals_(o.arrivals_),
        view_(o.view_)
    {}
    Frame& operator=(Frame const& o)
    {
        frameNumber_ = o.frameNumber_;
        universe_ = o.universe_;
        departures_ = o.departures_;
        arrivals_ = o.arrivals_;
        view_ = o.view_;
        return *this;
    }

    //These "correct" functions are for rearranging pointers when universes get copied.
    //Changes universe_ to &newUniverse
    void correctUniverse(Universe& newUniverse);

    //returns the frames whose arrivals are changed
    //newDeparture may get its contents pilfered
    FrameUpdateSet updateDeparturesFromHere(FrameDeparturesT& newDeparture);

    void setView(FrameView& newView) { view_.swap(newView); }
    FrameView const& getView() const { return view_; }
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
    Frame const *nextFrame(TimeDirection direction) const;
    Frame *nextFrame(TimeDirection direction);
    bool nextFrameInSameUniverse(TimeDirection direction) const;
    Universe const& getUniverse() const;
    Universe& getUniverse();
    int getFrameNumber() const;
    //<Valid to call with NullFrame>
    friend bool isNullFrame(Frame const *frame);
    //</Valid to call with NullFrame>
    //<Undefined to call with NullFrame>
    friend Frame const *nextFrame(Frame const *frame, TimeDirection direction);
    friend Frame *nextFrame(Frame* frame, TimeDirection direction);
    friend bool nextFrameInSameUniverse(Frame const *frame, TimeDirection direction);
    friend Universe& getUniverse(Frame *frame);
    friend Universe const& getUniverse(Frame const *frame);
    friend int getFrameNumber(Frame const *frame);
    //</Undefined to call with NullFrame>
    void insertArrival(const tbb::concurrent_hash_map<Frame const *, ObjectList<Normal> const *>::value_type& toInsert);
    void changeArrival(const tbb::concurrent_hash_map<Frame const *, ObjectList<Normal> const *>::value_type& toChange);
    void clearArrival(Frame const *toClear);

    /** Position of frame within universe_ */
    int frameNumber_;
    /** Back-link to universe which this frame is in */
    Universe *universe_;

    //Arrival departure map stuff. Could instead be put in external hash-map keyed by Frame*
    FrameDeparturesT departures_;
    tbb::concurrent_hash_map<Frame const *, ObjectList<Normal> const *> arrivals_;

    FrameView view_;
};

inline void swap(Frame& l, Frame& r)
{
    Frame temp(boost::move(l));
    l = boost::move(r);
    r = boost::move(temp);
}
}
#endif //HG_FRAME_H
