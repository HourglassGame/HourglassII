#ifndef HG_PHYSICS_ENGINE_H
#define HG_PHYSICS_ENGINE_H

#include "InputList.h"
#include "ObjectList.h"
#include "ObjectPtrList.h"
#include "ObjectListTypes.h"
#include "TriggerSystem.h"
#include "Environment.h"
#include "multi_thread_allocator.h"
#include "FrameView.h"
#include "OperationInterrupter.h"

#include <vector>
#include "mt/std/map"
#include "mt/std/vector"
#include <utility>

#include "Frame_fwd.h"
namespace hg {
class PhysicsEngine
{
public:
    PhysicsEngine(
        Environment const &env,
        TriggerSystem const &newTriggerSystem);


    typedef mt::std::map<Frame*, ObjectList<Normal> > FrameDepartureT;
    
    struct PhysicsReturnT
    {
    	FrameDepartureT departures;
        FrameView view;
    	bool currentPlayerFrame;
    	bool nextPlayerFrame;
    	bool currentWinFrame;
    };

    // executes frame and returns departures
    PhysicsEngine::PhysicsReturnT executeFrame(
        ObjectPtrList<Normal> const &arrivals,
        Frame *frame,
        std::vector<InputList> const &playerInput,
        OperationInterrupter &interrupter) const;
private:
    Environment env_;
    TriggerSystem triggerSystem_;
};
inline void swap(PhysicsEngine &l, PhysicsEngine &r)
{
    PhysicsEngine temp(boost::move(l));
    l = boost::move(r);
    r = boost::move(temp);
}
}//namespace hg
#endif //HG_PHYSICS_ENGINE_H
