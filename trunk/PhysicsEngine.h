#ifndef HG_PHYSICS_ENGINE_H
#define HG_PHYSICS_ENGINE_H

#include "InputList.h"
#include "ObjectList.h"
#include "ObjectPtrList.h"
#include "ObjectListTypes.h"
#include "TriggerSystem.h"
#include "Environment.h"
#include "multi_thread_allocator.h"

#include <vector>
#include "mt/std/map"
#include "mt/std/vector"

#include "Frame_fwd.h"
namespace hg {
class PhysicsEngine
{
public:
    PhysicsEngine(
        Environment const& env,
        TriggerSystem const& newTriggerSystem);
    typedef mt::std::map<Frame*, ObjectList<Normal> >::type FrameDepartureT;
    typedef mt::std::vector<RectangleGlitz>::type NewGlitzType;
    struct PhysicsReturnT
    {
    	PhysicsReturnT(
            FrameDepartureT const& Ndepartures,
            NewGlitzType const& Nglitz,
            bool NcurrentPlayerFrame,
    		bool NnextPlayerFrame,
    		bool NcurrentWinFrame) :
                departures(Ndepartures),
                glitz(Nglitz),
                currentPlayerFrame(NcurrentPlayerFrame),
                nextPlayerFrame(NnextPlayerFrame),
                currentWinFrame(NcurrentWinFrame)
    	{}
    	FrameDepartureT departures;
    	NewGlitzType glitz;
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
    TriggerSystem triggerSystem_;
};
}//namespace hg
#endif //HG_PHYSICS_ENGINE_H
