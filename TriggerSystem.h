#ifndef HG_TRIGGER_SYSTEM_H
#define HG_TRIGGER_SYSTEM_H

#include "Button.h"
#include "PlatformDestination.h"
#include "AttachmentMap.h"
#include "ObjectList.h"
#include "InputList.h"
#include "Frame_fwd.h"
#include <vector>
namespace hg
{
//definition: Statically identifiable object -- object which never arrives more than once in a particular frame
//                                              [note: which therefore can be unabiguouly identified by its index]
//A pluggable component for the physics engine which allows arbitrary code to be executed to calculate the departures
//of staticly identifiable objects based on arrivals to a frame. Also has capabilites for adding extra "TriggerObjects",
//which allow it to communicate with itself through time.
class TriggerSystem {
public:
//The current version is a crappified and cleaned-up version of the original, which also include all the static object 
//physics from the physics engine.
//The old model was "TriggerSystem is a system for combining button states into platform destinations"
//The old code did not actually manage that, but I can only assume it was an implementation issue.
//New model is described above. The new model may be overly general, but it can be cut back as required.
//This particular incarnation requires an exact 1 to 1 to 1 relationship between buttons, platforms and destinations.
//That is: each button corresponds to a pair of destinations, which is selected between when doing platform physics
//This version can, however, handle the situation where objects arrivals have not yet arrived, and so 
//particular platforms/buttons/etc have not yet come into existence.
TriggerSystem(
    unsigned int buttonCount,
    const std::vector<PlatformDestinationPair>& destinations,
    const AttachmentMap& attachmentMap);


//Creates Departures for all arrivals of pauseLevel == 0 staticly identifiable objects in arrivals
//These objects must also be in the top-level universe (as otherwise they would not be statically identifiable)
//Therefore, if !(getInitiatorFrame(getUniverse(time))) then this returns a map which contains no statically identifiable object departures.
//Also creates TriggerObjects. These are a trigger ID combined with arbitrary data 
//(stored as an int for now, could be changed to be something else if necessecary), which the time engine simply makes depart.
//Could also return a special ObjectList-ish thing which contains instructions for physics (such as, "create pause here" etc.)
//This would, however, complicate the interface, so I'd like to not do it unless there is a specific reason/level that
//needs the feature.
//Note: The time-engine is still responsible for adding the "pause-time departures" of objects with pauseLevel != 0.
ObjectList calculateStaticDepartures(
    const ObjectPtrList& arrivals,
    const std::vector<InputList>& playerInput,
    const Frame* time) const;
private:
    unsigned int buttonCount_;
    std::vector<PlatformDestinationPair> destinations_;
    AttachmentMap attachmentMap_;
};
}
#endif //HG_TRIGGER_SYSTEM_H
