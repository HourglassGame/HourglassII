#ifndef HG_LEVEL_H
#define HG_LEVEL_H
#include "ObjectList.h"
#include "ObjectListTypes.h"
#include "FrameID.h"
#include "AttachmentMap.h"
#include "TriggerSystem.h"
#include "Environment.h"

namespace hg {
class Level {
//Probably will make these private when the level editor gets implemented
public:
    Level(
        unsigned nspeedOfTime,
        std::size_t ntimeLineLength,
        Environment const& nenvironment,
        ObjectList<Normal> const& ninitialObjects,
        FrameID const& nguyStartTime,
        TriggerSystem const& ntriggerSystem);

    unsigned speedOfTime;
    std::size_t timeLineLength;
    Environment environment;
    ObjectList<Normal> initialObjects;
    FrameID guyStartTime;
    TriggerSystem triggerSystem;
};
}
#endif //HG_LEVEL_H