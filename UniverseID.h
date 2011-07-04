#ifndef HG_UNIVERSE_ID_H
#define HG_UNIVERSE_ID_H

#include <boost/range.hpp>
#include <boost/operators.hpp>
#include <vector>

#include "FrameID_fwd.h"
#include "Universe_fwd.h"

namespace hg {
//Uniquely identifies a particular universe.
struct UniverseID {
    //top level universeID
    //timelineLength is always length of top-level universe
    explicit UniverseID(std::size_t timelineLength);

    //returns the length of the lowest level of this universe
    std::size_t timelineLength() const;

    bool operator==(const UniverseID& other) const;
    bool operator<(const UniverseID& other) const;
private:
    friend class Universe;
    friend class FrameID;
    friend std::size_t hash_value(const UniverseID& toHash);
    //timelineLength_ -- length of the universe.
    std::size_t timelineLength_;
};
}
#endif //HG_UNIVERSE_ID_H
