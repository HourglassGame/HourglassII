#ifndef HG_GUY_OUTPUT_INFO_H
#define HG_GUY_OUTPUT_INFO_H
#include <cstddef>
#include "TimeDirection.h"
#include "Ability.h"
#include "mt/std/map"
namespace hg {
class GuyOutputInfo {
public:
    explicit GuyOutputInfo(TimeDirection timeDirection, mt::std::map<Ability, int>::type const& pickups) :
        timeDirection_(timeDirection), pickups_(pickups) {}
    TimeDirection getTimeDirection() const { return timeDirection_; }
    mt::std::map<Ability, int>::type const& getPickups() const { return pickups_; }
private:
    TimeDirection timeDirection_;
    mt::std::map<Ability, int>::type pickups_;
};
}//namespace hg
#endif //HG_GUY_OUTPUT_INFO_H
