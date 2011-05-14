#include "Level.h"
namespace hg {
Level::Level(unsigned int nspeedOfTime,
             std::size_t ntimeLineLength,
             const Environment& nenvironment,
             const ObjectList<Normal> & ninitialObjects,
             const FrameID& nguyStartTime,
             const TriggerSystem& ntriggerSystem) :
        speedOfTime(nspeedOfTime),
        timeLineLength(ntimeLineLength),
        environment(nenvironment),
        initialObjects(ninitialObjects),
        guyStartTime(nguyStartTime),
        triggerSystem(ntriggerSystem)
{
}
}
