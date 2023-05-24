#ifndef HG_LEVEL_H
#define HG_LEVEL_H
#include "ArrivalDepartures/ObjectList.h"
#include "ArrivalDepartures/ObjectListTypes.h"
#include "FrameID.h"
#include "hg/PhysicsEngine/TriggerSystem/TriggerSystem.h"
#include "hg/PhysicsEngine/Environment.h"
#include <tuple>
namespace hg {
struct Level final {
//Probably will make these private when the level editor gets implemented
	unsigned speedOfTime;
	unsigned speedOfTimeFuture;
	int timelineLength;
	Environment environment;
	ObjectList<NonGuyDynamic> initialObjects;
	Guy initialGuy;
	FrameID guyStartTime;
	TriggerSystem triggerSystem;

	bool operator==(Level const& o) const noexcept = default;
};
}
#endif //HG_LEVEL_H
