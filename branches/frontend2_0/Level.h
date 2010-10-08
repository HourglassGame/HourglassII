#ifndef HG_LEVEL_H
#define HG_LEVEL_H
#include "ObjectList.h"
#include "NewFrameID.h"
#include "AttachmentMap.h"
#include "TriggerSystem.h"
#include <vector>

namespace hg {
class Level {
//Probably will make these private when the level editor gets implemented
public:
    unsigned int speedOfTime;
    unsigned int timeLineLength;
    ::std::vector< ::std::vector<bool> > wallmap;
    int wallSize;
    int gravity;
    ObjectList initialObjects;
    NewFrameID guyStartTime;
    AttachmentMap attachmentMap;
    TriggerSystem triggerSystem;
};
}
#endif //HG_LEVEL_H
