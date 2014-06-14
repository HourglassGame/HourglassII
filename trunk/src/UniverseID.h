#ifndef HG_UNIVERSE_ID_H
#define HG_UNIVERSE_ID_H

#include <boost/range.hpp>
#include <boost/operators.hpp>
#include <istream>
#include <ostream>
#include <vector>

#include "TimeDirection.h"

#include "FrameID_fwd.h"
#include "UniverseID_fwd.h"

namespace boost {
    namespace serialization {
        class access;
    }
}

namespace hg {
std::ostream &operator<<(std::ostream &os, UniverseID const &toPrint);
std::istream &operator>>(std::istream &is, UniverseID &toRead);

FrameID getEntryFrame(UniverseID const &universe, TimeDirection direction);
FrameID getArbitraryFrame(UniverseID const &universe, int frameNumber);
int getTimelineLength(UniverseID const &universe);
std::size_t hash_value(UniverseID const &toHash);
//Uniquely identifies a particular universe.
class UniverseID {
public:
    //top level universeID
    //timelineLength is always length of top-level universe
    explicit UniverseID(int timelineLength);

    bool operator==(UniverseID const &o) const;
    bool operator <(UniverseID const &o) const;
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, unsigned int const version)
    {
        (void)version;
        ar & timelineLength;
    }
    
    friend std::ostream &operator<<(std::ostream &os, UniverseID const &toPrint);
    friend std::istream &operator>>(std::istream &is, UniverseID &toRead);

    friend FrameID getEntryFrame(UniverseID const &universe, TimeDirection direction);
    friend FrameID getArbitraryFrame(UniverseID const &universe, int frameNumber);
    friend int getTimelineLength(UniverseID const &universe);

    friend std::size_t hash_value(UniverseID const &toHash);
    int timelineLength;
};

inline std::ostream &operator<<(std::ostream &os, UniverseID const &toPrint)
{
    os << toPrint.timelineLength;
    return os;
}
inline std::istream &operator>>(std::istream &is, UniverseID &toRead)
{
    is >> toRead.timelineLength;
    return is;
}

}
#endif //HG_UNIVERSE_ID_H
