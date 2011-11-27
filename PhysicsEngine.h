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
    PhysicsEngine(PhysicsEngine&&) = default;
    PhysicsEngine& operator=(PhysicsEngine&&) = default;
    typedef mt::std::map<Frame*, ObjectList<Normal> >::type FrameDepartureT;
    typedef mt::std::vector<RectangleGlitz>::type NewGlitzType;
    struct PhysicsReturnT
    {
    	PhysicsReturnT(
            FrameDepartureT const& Ndepartures,
            FrameView const& Nview,
            bool NcurrentPlayerFrame,
    		bool NnextPlayerFrame,
    		bool NcurrentWinFrame) :
                departures(Ndepartures),
                view(Nview),
                currentPlayerFrame(NcurrentPlayerFrame),
                nextPlayerFrame(NnextPlayerFrame),
                currentWinFrame(NcurrentWinFrame)
    	{}
    	FrameDepartureT departures;
        FrameView view;
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
