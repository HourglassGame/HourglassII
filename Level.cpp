#include "Level.h"
namespace hg {
Level::Level(
    unsigned nspeedOfTime,
    std::size_t ntimeLineLength,
    Environment const& nenvironment,
    ObjectList<Normal> const& ninitialObjects,
    FrameID const& nguyStartTime,
    NewOldTriggerSystem const& nnewOldTriggerSystem) :
        speedOfTime(nspeedOfTime),
        timeLineLength(ntimeLineLength),
        environment(nenvironment),
        initialObjects(ninitialObjects),
        guyStartTime(nguyStartTime),
        newOldTriggerSystem(nnewOldTriggerSystem)
{
}
}
