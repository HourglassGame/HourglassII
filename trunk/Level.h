#ifndef HG_LEVEL_H
#define HG_LEVEL_H
#include "ObjectList.h"
#include "FrameID.h"
#include "AttachmentMap.h"
#include "TriggerSystem.h"
#include <boost/multi_array.hpp>

namespace hg {
class Level {
//Probably will make these private when the level editor gets implemented
public:
Level(unsigned int nspeedOfTime,
    size_t ntimeLineLength,
    const boost::multi_array<bool, 2>& nwallmap,
    int nwallSize,
    int ngravity,
    const ObjectList& ninitialObjects,
    const FrameID& nguyStartTime,
    const AttachmentMap& nattachmentMap,
    const TriggerSystem& ntriggerSystem);
    
    unsigned int speedOfTime;
    size_t timeLineLength;
    boost::multi_array<bool, 2> wallmap;
    int wallSize;
    int gravity;
    ObjectList initialObjects;
    FrameID guyStartTime;
    AttachmentMap attachmentMap;
    TriggerSystem triggerSystem;
};
}
#endif //HG_LEVEL_H
