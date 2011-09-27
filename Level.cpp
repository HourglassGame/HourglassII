#include "Level.h"
namespace hg {
Level::Level(
    unsigned nspeedOfTime,
    std::size_t ntimelineLength,
    Environment const& nenvironment,
    ObjectList<NonGuyDynamic> const& ninitialObjects,
    Guy const& ninitialGuy,
    FrameID const& nguyStartTime,
    TriggerSystem const& ntriggerSystem) :
        speedOfTime(nspeedOfTime),
        timelineLength(ntimelineLength),
        environment(nenvironment),
        initialObjects(ninitialObjects),
        initialGuy(ninitialGuy),
        guyStartTime(nguyStartTime),
        triggerSystem(ntriggerSystem)
{
}
}
