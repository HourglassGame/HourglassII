#include "Level.h"
#include <utility>
#include "move.h"
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
        environment(hg::move(nenvironment)),
        initialObjects(hg::move(ninitialObjects)),
        initialGuy(hg::move(ninitialGuy)),
        guyStartTime(hg::move(nguyStartTime)),
        triggerSystem(hg::move(ntriggerSystem))
{
}
}
