#ifndef HG_LEVEL_H
#define HG_LEVEL_H
#include "ObjectList.h"
#include "ObjectListTypes.h"
#include "FrameID.h"
#include "TriggerSystem.h"
#include "Environment.h"
#include <tuple>
namespace hg {
class Level {
private:
    auto equality_tuple() const -> decltype(auto)
    {
        return std::tie(
            speedOfTime,
            timelineLength,
            environment,
            initialObjects,
            initialGuy,
            guyStartTime,
            triggerSystem);
    }
//Probably will make these private when the level editor gets implemented
public:
    unsigned speedOfTime;
    int timelineLength;
    Environment environment;
    ObjectList<NonGuyDynamic> initialObjects;
    Guy initialGuy;
    FrameID guyStartTime;
    TriggerSystem triggerSystem;

    bool operator==(Level const &o) const
    {
        return equality_tuple() == o.equality_tuple();
    }
};
}
#endif //HG_LEVEL_H
