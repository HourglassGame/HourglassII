#include "Level.h"
namespace hg {
Level::Level(unsigned int nspeedOfTime,
             unsigned int ntimeLineLength,
             const ::boost::multi_array<bool, 2>& nwallmap,
             int nwallSize,
             int ngravity,
             const ObjectList& ninitialObjects,
             const NewFrameID& nguyStartTime,
             const AttachmentMap& nattachmentMap,
             const TriggerSystem& ntriggerSystem) :
    speedOfTime(nspeedOfTime),
    timeLineLength(ntimeLineLength),
    wallmap(nwallmap),
    wallSize(nwallSize),
    gravity(ngravity),
    initialObjects(ninitialObjects),
    guyStartTime(nguyStartTime),
    attachmentMap(nattachmentMap),
    triggerSystem(ntriggerSystem)
    {
    }
}