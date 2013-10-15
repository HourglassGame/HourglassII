#ifndef HG_GUY_OUTPUT_INFO_H
#define HG_GUY_OUTPUT_INFO_H
#include <cstddef>
#include "TimeDirection.h"
#include "Ability.h"
#include "mt/std/map"
namespace hg {
class GuyOutputInfo {
public:
    explicit GuyOutputInfo(std::size_t index, TimeDirection timeDirection, mt::std::map<Ability, int>::type const &pickups, bool boxCarrying, TimeDirection boxCarryDirection, int x, int y) :
        index(index), timeDirection(timeDirection), pickups(pickups), boxCarrying(boxCarrying), boxCarryDirection(boxCarryDirection), x(x), y(y) {}
    std::size_t getIndex() const { return index; }
    TimeDirection getTimeDirection() const { return timeDirection; }
    mt::std::map<Ability, int>::type const &getPickups() const { return pickups; }
    bool getBoxCarrying() const { return boxCarrying; }
    TimeDirection getBoxCarryDirection() const { return boxCarryDirection; }
    int getX() const { return x; }
    int getY() const { return y; }
private:
    std::size_t index;
    TimeDirection timeDirection;
    mt::std::map<Ability, int>::type pickups;
    bool boxCarrying;
    TimeDirection boxCarryDirection;
    int x;
    int y;
};
}//namespace hg
#endif //HG_GUY_OUTPUT_INFO_H
