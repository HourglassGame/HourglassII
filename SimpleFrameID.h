#ifndef HG_SIMPLE_FRAME_ID_H
#define HG_SIMPLE_FRAME_ID_H

#include "TimeDirection.h"

#include <boost/functional/hash.hpp>

#include <cassert>

namespace hg {
//Holds timelineLength and frame, this is what I should have done originally instead of that typedef.
//If all goes to plan this will properly integrate and pave the way to NewFrameID
class SimpleFrameID {
public:
    //creates a nullframe
    SimpleFrameID();
        
    SimpleFrameID(unsigned int time, unsigned int timelineLength);
      
    // returns the normal next frame for things moving in direction TimeDirection
    SimpleFrameID nextFrame(TimeDirection direction) const;
    //Wrapper for nextFrame to allow existing code to work without change. Remove if you don't feel the need.
    SimpleFrameID operator+(TimeDirection direction) const
    {
        return nextFrame(direction);
    }    
    // returns if the next frame for things moving in direction TimeDirection
    //is part of the same pause time universe as the frame
    bool nextFrameInUniverse(TimeDirection direction) const;
    
    // returns a frameID using frameNumber as 'distance' from the start of the universe in the same universe as *this
    SimpleFrameID arbitraryFrameInUniverse(unsigned int frameNumber) const;
        
    bool operator==(const SimpleFrameID& other) const;
        
    bool operator<(const SimpleFrameID& other) const;
        
    bool isValidFrame() const;

    unsigned int frame() const 
    {
        assert (isValidFrame());
        return frame_;
    }

private:
    friend ::std::size_t hash_value(const SimpleFrameID& toHash);
    unsigned int frame_; 
    unsigned int timelineLength_;
};
::std::size_t hash_value(const SimpleFrameID& toHash);
}//namespace hg

#endif //HG_SIMPLE_FRAME_ID_H
