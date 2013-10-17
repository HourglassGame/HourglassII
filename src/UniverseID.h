#ifndef HG_UNIVERSE_ID_H
#define HG_UNIVERSE_ID_H

#include <boost/range.hpp>
#include <boost/operators.hpp>
#include <istream>
#include <ostream>
#include <vector>

#include "TimeDirection.h"

#include "FrameID_fwd.h"

namespace boost {
    namespace serialization {
        class access;
    }
}

namespace hg {
//Uniquely identifies a particular universe.
class UniverseID {
public:
    //top level universeID
    //timelineLength is always length of top-level universe
    explicit UniverseID(int timelineLength);

    //returns the length of the lowest level of this universe
    int timelineLength() const;

    bool operator==(UniverseID const &o) const;
    bool operator <(UniverseID const &o) const;
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, unsigned int const version)
    {
        (void)version;
        ar & timelineLength_;
    }
        inline friend std::ostream &operator<<(std::ostream &os, UniverseID const &toPrint)
    {
        os << toPrint.timelineLength_;
        return os;
    }
    inline friend std::istream &operator>>(std::istream &is, UniverseID &toRead)
    {
        is >> toRead.timelineLength_;
        return is;
    }

    friend FrameID getEntryFrame(UniverseID const &universe, TimeDirection direction);
    friend FrameID getArbitraryFrame(UniverseID const &universe, int frameNumber);
    friend int getTimelineLength(UniverseID const &universe);

    friend class FrameID;
    friend std::size_t hash_value(UniverseID const &toHash);
    //timelineLength_ -- length of the universe.
    int timelineLength_;
};
}
#endif //HG_UNIVERSE_ID_H
