#ifndef HG_PHYSICS_ENGINE_H
#define HG_PHYSICS_ENGINE_H

#include "InputList.h"
#include "ObjectList.h"
#include "TriggerSystem.h"
#include "RemoteDepartureEdit.h"
#include "Environment.h"

#include <vector>
#include <map>

#include "Frame_fwd.h"
namespace hg {
class PhysicsEngine
{
public:
    PhysicsEngine(
        const Environment& env,
        const TriggerSystem& triggerSystem);

    struct PhysicsReturnT
    {
    	PhysicsReturnT(
            const std::map<Frame*, ObjectList>& Ndepartures,
            bool NcurrentPlayerFrame,
    		bool NnextPlayerFrame,
    		bool NcurrentWinFrame) :
                departures(Ndepartures),
                currentPlayerFrame(NcurrentPlayerFrame),
                nextPlayerFrame(NnextPlayerFrame),
                currentWinFrame(NcurrentWinFrame)
    	{}
    	std::map<Frame*, ObjectList> departures;
    	bool currentPlayerFrame;
    	bool nextPlayerFrame;
    	bool currentWinFrame;
    };

    // executes frame and returns departures
    PhysicsEngine::PhysicsReturnT executeFrame(
        const ObjectPtrList& arrivals,
        Frame* time,
        const std::vector<InputList>& playerInput) const;
private:
    Environment env_;
    TriggerSystem triggerSystem_;
};
}
#endif //HG_PHYSICS_ENGINE_H
