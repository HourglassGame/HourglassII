#include "Level.h"
#include <utility>
namespace hg {
Level::Level(
    unsigned nspeedOfTime,
    int ntimelineLength,
    BOOST_RV_REF(Environment) nenvironment,
    BOOST_RV_REF(ObjectList<NonGuyDynamic>) ninitialObjects,
    Guy const& ninitialGuy,
    FrameID const& nguyStartTime,
    BOOST_RV_REF(TriggerSystem) ntriggerSystem) :
        speedOfTime(nspeedOfTime),
        timelineLength(ntimelineLength),
        environment(boost::move(nenvironment)),
        initialObjects(boost::move(ninitialObjects)),
        initialGuy(ninitialGuy),
        guyStartTime(nguyStartTime),
        triggerSystem(boost::move(ntriggerSystem))
{
}
}
