#include "Level.h"
#include <utility>
namespace hg {
Level::Level(
    unsigned nspeedOfTime,
    std::size_t ntimelineLength,
    Environment&& nenvironment,
    ObjectList<NonGuyDynamic>&& ninitialObjects,
    Guy&& ninitialGuy,
    FrameID&& nguyStartTime,
    TriggerSystem&& ntriggerSystem) :
        speedOfTime(nspeedOfTime),
        timelineLength(ntimelineLength),
        environment(std::move(nenvironment)),
        initialObjects(std::move(ninitialObjects)),
        initialGuy(std::move(ninitialGuy)),
        guyStartTime(std::move(nguyStartTime)),
        triggerSystem(std::move(ntriggerSystem))
{
}
}
