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
#include "mt/boost/container/map.hpp"
#include "mt/std/vector"
#include <utility>
#include <boost/move/move.hpp>

#include "Frame_fwd.h"
namespace hg {
class PhysicsEngine
{
public:
    PhysicsEngine(
        Environment const& env,
        TriggerSystem const& newTriggerSystem);
    
    PhysicsEngine(PhysicsEngine const& other) :
        env_(other.env_),
        triggerSystem_(other.triggerSystem_)
    {
    }
    PhysicsEngine& operator=(BOOST_COPY_ASSIGN_REF(PhysicsEngine) other)
    {
        env_ = other.env_;
        triggerSystem_ = other.triggerSystem_;
        return *this;
    }
    
    PhysicsEngine(BOOST_RV_REF(PhysicsEngine) other) :
        env_(boost::move(other.env_)),
        triggerSystem_(boost::move(other.triggerSystem_))
    {}
    PhysicsEngine& operator=(BOOST_RV_REF(PhysicsEngine) other)
    {
        env_ = boost::move(other.env_);
        triggerSystem_ = boost::move(other.triggerSystem_);
        return *this;
    }
    typedef mt::boost::container::map<Frame*, ObjectList<Normal> >::type FrameDepartureT;
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
    BOOST_COPYABLE_AND_MOVABLE(PhysicsEngine)
};
inline void swap(PhysicsEngine& l, PhysicsEngine& r)
{
    PhysicsEngine temp(boost::move(l));
    l = boost::move(r);
    r = boost::move(temp);
}
}//namespace hg
#endif //HG_PHYSICS_ENGINE_H
