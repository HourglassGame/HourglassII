#ifndef HG_FRAME_ID_H
#define HG_FRAME_ID_H

#include "TimeDirection.h"
#include "UniverseID.h"
#include <boost/operators.hpp>
#include <tuple>

#include "Frame_fwd.h"

#include <cstddef>
#include <istream>
#include <ostream>

namespace boost {
    namespace serialization {
        class access;
    }
}

namespace hg {
bool isNullFrame(FrameID const &frame);
FrameID nextFrame(FrameID const &frame, TimeDirection direction);
bool nextFrameInSameUniverse(FrameID const &frame, TimeDirection direction);
UniverseID getUniverse(FrameID const &frame);
int getFrameNumber(FrameID const &frame);

std::ostream &operator<<(std::ostream &os, FrameID const &toPrint);
std::istream &operator>>(std::istream &is, FrameID &toRead);
//Class following original intention of FrameID. May be too slow for back-end use,
//Compliments Frame * by not requiring a central authority (ie the base universe) to be used
class FrameID final : boost::totally_ordered<FrameID>
{
public:
    //Creates a nullframe
    FrameID() noexcept;

    //Creates a FrameID referring to the given position in the given universe
    FrameID(int frameNumber, UniverseID const &universe);

    //Creates a FrameID corresponding to the given Frame*
    explicit FrameID(Frame const *toConvert);

    // returns the normal next frame for things moving in direction TimeDirection
    FrameID nextFrame(TimeDirection direction) const;
    bool nextFrameInSameUniverse(TimeDirection direction) const;

    // returns a frameID using frameNumber as 'distance' from the start of the universe in
    FrameID arbitraryFrameInUniverse(int frameNumber) const;

    bool operator==(FrameID const &o) const;
    bool operator<(FrameID const &o) const;

    bool isValidFrame() const;
    int getFrameNumber() const;
    UniverseID const &getUniverse() const;
private:
    friend bool isNullFrame(FrameID const &frame);
    friend FrameID nextFrame(FrameID const &frame, TimeDirection direction);
    friend bool nextFrameInSameUniverse(FrameID const &frame, TimeDirection direction);
    friend UniverseID getUniverse(FrameID const &frame);
    friend int getFrameNumber(FrameID const &frame);
    
    friend std::ostream &operator<<(std::ostream &os, FrameID const &toPrint);
    friend std::istream &operator>>(std::istream &is, FrameID &toRead);
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, unsigned int const version)
    {
        (void)version;
        ar & frame;
        ar & universeID;
    }
    //positiong of frame within universeID_
    int frame;
    UniverseID universeID;
    auto comparison_tuple() const -> decltype(auto) {
        return std::tie(frame, universeID);
    }
};

inline std::ostream &operator<<(std::ostream &os, FrameID const &toPrint)
{
    os << toPrint.frame << " ";
    os << toPrint.universeID;
    return os;
}
inline std::istream &operator>>(std::istream &is, FrameID &toRead)
{
    is >> toRead.frame;
    is >> toRead.universeID;
    return is;
}

//Returns a std::size_t based on toHash such that two FrameIDs for which operator== returns true give the same std::size_t value;
std::size_t hash_value(FrameID const &toHash);
}//namespace hg
#endif //HG_FRAME_ID_H
