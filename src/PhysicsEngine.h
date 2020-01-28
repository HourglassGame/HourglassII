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
#include "ThreadLocal.h"
#include "memory_pool.h"
#include "copy_as_new_ptr.h"

#include <vector>
#include "mt/std/map"
#include "mt/std/vector"
#include <utility>

#include "Frame_fwd.h"
namespace hg {
class PhysicsEngine final
{
public:
    PhysicsEngine(
        Environment const &env,
        TriggerSystem const &newTriggerSystem);

    typedef mt::std::map<Frame*, ObjectList<Normal>> FrameDepartureT;

    struct PhysicsReturnT
    {
        FrameDepartureT departures;
        FrameView view;
        mt::std::vector<std::tuple<std::size_t, Frame *>> guyDepartureFrames;
        int speedOfTime;
        bool currentWinFrame;
    };

    // executes frame and returns departures
    PhysicsEngine::PhysicsReturnT executeFrame(
        ObjectPtrList<Normal> const &arrivals,
        Frame *frame,
        std::vector<GuyInput> const &playerInput,
        OperationInterrupter &interrupter) const;
private:
    Environment env_;
    TriggerSystem triggerSystem_;
    //TODO: Allow memory_pool to be copied, to avoid use of copy_as_new_ptr?
#if USE_POOL_ALLOCATOR
    copy_as_new_ptr<ThreadLocal<memory_pool<user_allocator_tbb_alloc>>> shared_pool_;
#endif
};
inline void swap(PhysicsEngine &l, PhysicsEngine &r)
{
    PhysicsEngine temp(boost::move(l));
    l = boost::move(r);
    r = boost::move(temp);
}
}//namespace hg
#endif //HG_PHYSICS_ENGINE_H
