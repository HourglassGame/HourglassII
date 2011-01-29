#ifndef HG_FRAME_H
#define HG_FRAME_H

#include "TimeDirection.h"
#include "PauseInitiatorID.h"
#include "ObjectList.h"
#include <boost/unordered_map.hpp>
#include <map>
namespace hg {
class Frame;
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
    
    FramePtr(const Frame* ptr) :
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
/*
    //NullFrame construction
    Frame(unsigned int frameNumber, Universe* universe):
    frameNumber_(frameNumber),
    universe_(universe),
    departures_(),
    arrivals_(),
    subUniverses_()
    {
    }
    //assignment
    Frame& operator=(const FramePtr& other);
    
    //copy-construction
    Frame(const Frame& other) :
    frameNumber_(other.frameNumber_),
    universe_(other.universe_),
    departures_(other.departures_),
    arrivals_(other.arrivals_),
    subUniverses_(other.subUniverses_)
    {
    }
    
    // returns the normal next frame for things moving in direction TimeDirection
    Frame* nextFrame(TimeDirection direction) const
    {
        if(frameNumber_ + direction >= 0) {
            return universe->getArbitraryFrame(frameNumber_ + direction);
        }
        else {
            return 0;
        }
    }

    // returns if the next frame for things moving in direction TimeDirection
    //is part of the same pause time universe as the frame
    bool nextFramePauseLevelDifference(TimeDirection direction) const
    {
        
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
    //returns the frames whose arrivals are changed
    //newDeparture may get its contents pilfered
    template<typename Funcs>
    FrameUpdateSet updateDeparturesFromHere(std::map<Frame*, ObjectList>& newDeparture, Funcs changeArrivalFunc);
    
private:
    FramePtr& operator=(const FramePtr& other)
    unsigned int frameNumber_;
    //back-link to universe in which this frame is
    Universe* universe_;
    std::map<Frame*, ObjectList> departures_;
    std::map<Frame*, ObjectList*> arrivals_;
    ::boost::unordered_map<PauseInitiatorID, Universe> subUniverses_;*/
};
/*
template<typename Funcs>
inline FrameUpdateSet Frame::updateDeparturesFromHere(std::map<Frame*, ObjectList>& newDeparture, Funcs changeArrivalFuncs);
{
    FrameUpdateSet changedTimes;

    Iterator ni(newDeparture.begin());
    const Iterator nend(newDeparture.end());
    Iterator oi(departures_.begin());
    const Iterator oend(departures_.end());

    while (oi != oend) {
        while (true) {
            if (ni != nend) {
                if (ni->first < oi->first) {
                    Funcs.insert(ni->first, make_pair(this, &ni->second));
                    //arrivals.at(ni->first).insertObjectList(time, ni->second);
                    changedTimes.addFrame(ni->first);
                    ++ni;
                }
                else if (ni->first == oi->first) {
                    if (ni->second != oi->second) {
                        Funcs.change(ni->first, make_pair(this, &ni->second));
                        //arrivals[ni->first].setObjectList(time, ni->second);
                        changedTimes.addFrame(ni->first);
                    }
                    ++ni;
                    break;
                }
                else {
                    Funcs.clear(oi->first, this);
                    //arrivals[oi->first].clearTime(time);
                    changedTimes.addFrame(oi->first);
                    break;
                }
            }
            else {
                while (oi != oend) {
                    Funcs.clear(oi->first, this);
                    //arrivals[oi->first].clearTime(time);
                    changedTimes.addFrame(oi->first);
                    ++oi;
                }
                goto end;
            }
        }
        ++oi;
    }
    while (ni != nend) {
        Funcs.insert(ni->first, make_pair(this, &ni->second));
        //arrivals[ni->first].insertObjectList(time, ni->second);
        changedTimes.addFrame(ni->first);
        ++ni;
    }
end:
    departures[time].swap(newDeparture);
    return changedTimes;
}
*/
}
#endif //HG_FRAME_H
