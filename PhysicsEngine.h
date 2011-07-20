#ifndef HG_PHYSICS_ENGINE_H
#define HG_PHYSICS_ENGINE_H

#include "InputList.h"
#include "ObjectList.h"
#include "ObjectPtrList.h"
#include "ObjectListTypes.h"
#include "TriggerSystem.h"
#include "Environment.h"

#include <vector>
#include <map>

#include "Frame_fwd.h"
namespace hg {
class PhysicsEngine
{
public:
    PhysicsEngine(
        Environment const& env,
        NewOldTriggerSystem const& newTriggerSystem);

    struct PhysicsReturnT
    {
    	PhysicsReturnT(
            std::map<Frame*, ObjectList<Normal> > const& Ndepartures,
            std::vector<RectangleGlitz> const& Nglitz,
            bool NcurrentPlayerFrame,
    		bool NnextPlayerFrame,
    		bool NcurrentWinFrame) :
                departures(Ndepartures),
                glitz(Nglitz),
                currentPlayerFrame(NcurrentPlayerFrame),
                nextPlayerFrame(NnextPlayerFrame),
                currentWinFrame(NcurrentWinFrame)
    	{}
    	std::map<Frame*, ObjectList<Normal> > departures;
        std::vector<RectangleGlitz> glitz;
    	bool currentPlayerFrame;
    	bool nextPlayerFrame;
    	bool currentWinFrame;
    };

    // executes frame and returns departures
    PhysicsEngine::PhysicsReturnT executeFrame(
        ObjectPtrList<Normal> const& arrivals,
        Frame* time,
        std::vector<InputList> const& playerInput) const;
private:
    Environment env_;
    NewOldTriggerSystem newTriggerSystem_;
};
}//namespace hg
#endif //HG_PHYSICS_ENGINE_H
