#ifndef HG_LEVEL_H
#define HG_LEVEL_H
#include "ArrivalDepartures/ObjectList.h"
#include "ArrivalDepartures/ObjectListTypes.h"
#include "FrameID.h"
#include "hg/PhysicsEngine/TriggerSystem/TriggerSystem.h"
#include "hg/PhysicsEngine/Environment.h"
#include <tuple>
namespace hg {
class Level final {
private:
	auto equality_tuple() const -> decltype(auto)
	{
		return std::tie(
			speedOfTime,
			speedOfTimeFuture,
			timelineLength,
			environment,
			initialObjects,
			initialGuy,
			guyStartTime,
			triggerSystem);
	}
//Probably will make these private when the level editor gets implemented
public:
	unsigned speedOfTime;
	unsigned speedOfTimeFuture;
	int timelineLength;
	Environment environment;
	ObjectList<NonGuyDynamic> initialObjects;
	Guy initialGuy;
	FrameID guyStartTime;
	TriggerSystem triggerSystem;

	bool operator==(Level const &o) const
	{
		return equality_tuple() == o.equality_tuple();
	}
};
}
#endif //HG_LEVEL_H
