#include "Level.h"
namespace hg {
Level::Level(
    unsigned nspeedOfTime,
    std::size_t ntimeLineLength,
    Environment const& nenvironment,
    ObjectList<Normal> const& ninitialObjects,
    FrameID const& nguyStartTime,
    TriggerSystem const& ntriggerSystem) :
        speedOfTime(nspeedOfTime),
        timeLineLength(ntimeLineLength),
        environment(nenvironment),
        initialObjects(ninitialObjects),
        guyStartTime(nguyStartTime),
        triggerSystem(ntriggerSystem)
{
}
}
