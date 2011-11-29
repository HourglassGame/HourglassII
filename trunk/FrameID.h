#ifndef HG_FRAME_ID_H
#define HG_FRAME_ID_H

#include "TimeDirection.h"
#include "UniverseID.h"
#include <boost/operators.hpp>
#include "Frame_fwd.h"
#include "move.h"

#include <cstddef>
#include <istream>
#include <ostream>

namespace boost {
    namespace serialization {
        class access;
    }
}

namespace hg {
//Class following original intention of FrameID. May be too slow for back-end use,
//Compliments Frame* by not requiring a central authority (ie the base universe) to be used
class FrameID : boost::totally_ordered<FrameID>
{
public:
    //Creates a nullframe
    FrameID();

    //Creates a FrameID referring to the given time in the given universe
    FrameID(std::size_t time, const UniverseID& universe);

    //Creates a FrameID corresponding to the given Frame*
    explicit FrameID(const Frame* toConvert);

    // returns the normal next frame for things moving in direction TimeDirection
    FrameID nextFrame(TimeDirection direction) const;

    bool nextFrameInSameUniverse(TimeDirection direction) const;

    // returns a frameID using frameNumber as 'distance' from the start of the universe in
    FrameID arbitraryFrameInUniverse(std::size_t frameNumber) const;

    bool operator==(const FrameID& other) const;

    bool operator<(const FrameID& other) const;

    bool isValidFrame() const;

    std::size_t getFrameNumber() const;
    UniverseID const& getUniverse() const;
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        (void)version;
        ar & frame_;
        ar & universeID_;
    }
    inline friend std::ostream& operator<<(std::ostream& os, FrameID const& toPrint)
    {
        os << toPrint.frame_ << " ";
        os << toPrint.universeID_;
        return os;
    }
    inline friend std::istream& operator>>(std::istream& is, FrameID& toRead)
    {
        is >> toRead.frame_;
        is >> toRead.universeID_;
        return is;
    }
    //positiong of frame within universeID_
    std::size_t frame_;
    UniverseID universeID_;
};
//Returns a std::size_t based on toHash such that two FrameIDs for which operator== returns true give the same std::size_t value;
std::size_t hash_value(FrameID const& toHash);
inline void swap(FrameID& l, FrameID& r)
{
    FrameID temp(hg::move(l));
    l = hg::move(r);
    r = hg::move(temp);
}
}//namespace hg
#endif //HG_FRAME_ID_H
