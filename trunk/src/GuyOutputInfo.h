#ifndef HG_GUY_OUTPUT_INFO_H
#define HG_GUY_OUTPUT_INFO_H
#include <cstddef>
#include "TimeDirection.h"
#include "Ability.h"
#include "mt/std/map"
namespace hg {
class GuyOutputInfo {
public:
    explicit GuyOutputInfo(std::size_t index, TimeDirection timeDirection, mt::std::map<Ability, int>::type const& pickups, bool boxCarrying) :
        index(index), timeDirection(timeDirection), pickups(pickups), boxCarrying(boxCarrying) {}
    std::size_t getIndex() const { return index; }
    bool getBoxCarrying() const { return boxCarrying; }
    TimeDirection getTimeDirection() const { return timeDirection; }
    mt::std::map<Ability, int>::type const& getPickups() const { return pickups; }
private:
    std::size_t index;
    TimeDirection timeDirection;
    mt::std::map<Ability, int>::type pickups;
    bool boxCarrying;
};
}//namespace hg
#endif //HG_GUY_OUTPUT_INFO_H
