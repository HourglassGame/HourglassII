#ifndef HG_LEVEL_H
#define HG_LEVEL_H
#include "ObjectList.h"
#include "ObjectListTypes.h"
#include "FrameID.h"
#include "AttachmentMap.h"
#include "TriggerSystem.h"
#include "Environment.h"
#include <boost/move/move.hpp>
namespace hg {
class Level {
//Probably will make these private when the level editor gets implemented
public:
    Level(
        unsigned nspeedOfTime,
        int ntimelineLength,
        BOOST_RV_REF(Environment) nenvironment,
        BOOST_RV_REF(ObjectList<NonGuyDynamic>) ninitialObjects,
        Guy const &ninitialGuy,
        FrameID const &nguyStartTime,
        BOOST_RV_REF(TriggerSystem) ntriggerSystem);

    Level(Level const &o) :
        speedOfTime(o.speedOfTime),
        timelineLength(o.timelineLength),
        environment(o.environment),
        initialObjects(o.initialObjects),
        initialGuy(o.initialGuy),
        guyStartTime(o.guyStartTime),
        triggerSystem(o.triggerSystem)
    {}
    Level &operator=(BOOST_COPY_ASSIGN_REF(Level) o)
    {
        return *this = Level(o);
    }
    Level(BOOST_RV_REF(Level) o) :
        speedOfTime(boost::move(o.speedOfTime)),
        timelineLength(boost::move(o.timelineLength)),
        environment(boost::move(o.environment)),
        initialObjects(boost::move(o.initialObjects)),
        initialGuy(boost::move(o.initialGuy)),
        guyStartTime(boost::move(o.guyStartTime)),
        triggerSystem(boost::move(o.triggerSystem))
    {}
    Level &operator=(BOOST_RV_REF(Level) o)
    {
        speedOfTime = boost::move(o.speedOfTime);
        timelineLength = boost::move(o.timelineLength);
        environment = boost::move(o.environment);
        initialObjects = boost::move(o.initialObjects);
        initialGuy = boost::move(o.initialGuy);
        guyStartTime = boost::move(o.guyStartTime);
        triggerSystem = boost::move(o.triggerSystem);
        return *this;
    }
    unsigned speedOfTime;
    int timelineLength;
    Environment environment;
    ObjectList<NonGuyDynamic> initialObjects;
    Guy initialGuy;
    FrameID guyStartTime;
    TriggerSystem triggerSystem;
private:
    BOOST_COPYABLE_AND_MOVABLE(Level)
};
}
#endif //HG_LEVEL_H
