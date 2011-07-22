#include "Level.h"
namespace hg {
Level::Level(
    unsigned nspeedOfTime,
    std::size_t ntimeLineLength,
    BOOST_RV_REF(Environment) nenvironment,
    BOOST_RV_REF(ObjectList<Normal>) ninitialObjects,
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
