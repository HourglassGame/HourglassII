#ifndef HG_SIMPLE_CONFIGURED_TRIGGER_SYSTEM_H
#define HG_SIMPLE_CONFIGURED_TRIGGER_SYSTEM_H
#include "TriggerSystemImplementation.h"
#include "copy_as_new_ptr.h"
#include "SimpleLuaCpp.h"
#include "ThreadLocal.h"
#include "ObjectAndTime.h"
#include "LuaInterruption.h"
#include "LuaModule.h"
#include "memory_pool.h"
#include <string>
#include <vector>
#include <mutex>
#include "mp/std/map"
#include <boost/optional.hpp>
#include <boost/polymorphic_cast.hpp>
#include <tuple>
namespace hg {
    class OperationInterrupter;

    struct AxisCollisionDestination {
        int desiredPosition;
        int acceleration;
        int deceleration;
        int maxSpeed;
    };
    struct CollisionDestination {
        AxisCollisionDestination xDestination;
        AxisCollisionDestination yDestination;
    };
    struct ProtoCollision{
        TimeDirection timeDirection;
        int width;
        int height;
        CollisionDestination onDestination;
        CollisionDestination offDestination;
        //TODO: More elegant handling of hasButtonTriggerID
        bool hasButtonTriggerID;
        int buttonTriggerID;
        int lastStateTriggerID;
    };
    struct Attachment {
        bool hasPlatform;
        int platform;
        int xOffset;
        int yOffset;
    };
    struct ProtoPortal {
        Attachment attachment;
        int index;
        int xaim;
        int yaim;
        int width;
        int height;
        int collisionOverlap;
        TimeDirection timeDirection;
        int destinationIndex;
        int xDestination;
        int yDestination;
        bool relativeTime;
        int timeDestination;
        bool relativeDirection;
        TimeDirection destinationDirection;
        int illegalDestination;
        bool fallable;
        bool isLaser;
        bool winner;
    };
    struct ButtonFrameState {
        void calcPnV(mp::std::vector<Collision> const &collisions) {
            //TODO
        }
        void updateState(
            mt::std::map<Frame*, ObjectList<Normal>> const &departures,
            mp::std::vector<mp::std::vector<int>> const &triggerArrivals)
        {
            //TODO
        }
        void fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const/*?*/{
            //TODO
        }
    };
    struct ProtoButton {
        //TODO
        ButtonFrameState getFrameState() const {
            return {};
        }
    };
    
    struct MutatorFrameState {
        void addMutator(
            mp::std::vector<mp::std::vector<int>> const &triggerArrivals,
            mp::std::vector<Collision> const &collisions,
            mp::std::vector<MutatorArea> &mutators,
            mp::std::vector<MutatorFrameState *> &activeMutators
        )/* const ?*/
        {}

        void fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const/*?*/{
            //TODO
        }
        boost::optional<Guy> effect(Guy const &guy) const {
            return {guy};
        }
        boost::optional<Box> effect(Box const &box) const {
            return {box};
        }
    };
    struct ProtoMutator {
        //TODO
        MutatorFrameState getFrameState() const{
            return {};
        }
    };
    
    class SimpleConfiguredTriggerFrameState final :
        public TriggerFrameStateImplementation
    {
    public:
        SimpleConfiguredTriggerFrameState(
            std::vector<
                std::pair<
                    int,
                    std::vector<int>
                >
            > const &triggerOffsetsAndDefaults,
            std::size_t arrivalLocationsSize,
            std::vector<ProtoCollision> const &protoCollisions,
            std::vector<ProtoPortal> const &protoPortals,
            std::vector<ProtoButton> const &protoButtons,
            std::vector<ProtoMutator> const &protoMutators,
            memory_pool<user_allocator_tbb_alloc> &pool,
            OperationInterrupter &interrupter);

        virtual PhysicsAffectingStuff
            calculatePhysicsAffectingStuff(
                Frame const *currentFrame,
                boost::transformed_range<
                GetBase<TriggerDataConstPtr>,
                mt::boost::container::vector<TriggerDataConstPtr> const> const &triggerArrivals) override;

        virtual bool shouldArrive(Guy const &potentialArriver) override;
        virtual bool shouldArrive(Box const &potentialArriver) override;

        virtual bool shouldPort(
            int responsiblePortalIndex,
            Guy const &potentialPorter,
            bool porterActionedPortal) override;
        virtual bool shouldPort(
            int responsiblePortalIndex,
            Box const &potentialPorter,
            bool porterActionedPortal) override;

        virtual boost::optional<Guy> mutateObject(
            mp::std::vector<int> const &responsibleMutatorIndices,
            Guy const &objectToManipulate) override;
        virtual boost::optional<Box> mutateObject(
            mp::std::vector<int> const &responsibleMutatorIndices,
            Box const &objectToManipulate) override;

        virtual DepartureInformation getDepartureInformation(
            mt::std::map<Frame*, ObjectList<Normal>> const &departures,
            Frame *currentFrame) override;
        virtual ~SimpleConfiguredTriggerFrameState() noexcept override;
    private:
        memory_pool<user_allocator_tbb_alloc> &pool_;
        OperationInterrupter &interrupter_;
        //LuaState &L_;

        std::vector<
            std::pair<
            int,
            std::vector<int>
            >
        > const &triggerOffsetsAndDefaults_;
        //Gives the size that arrivalLocations must always be,
        //for script-validation purposes.
        //If a portal specifies an destinationIndex that is
        //larger than this, it is an error in the script and will be detected as such.
        std::size_t arrivalLocationsSize_;
        //TODO: Make this be a map, rather than a vector!
        mp::std::map<std::size_t, mt::std::vector<int>> outputTriggers_;
        mt::std::vector<Glitz> forwardsGlitz_;
        mt::std::vector<Glitz> reverseGlitz_;
        mp::std::vector<ButtonFrameState> buttonFrameStates_;
        mp::std::vector<MutatorFrameState> mutatorFrameStates_;
        mp::std::vector<MutatorFrameState *> activeMutators_;
        mp::std::vector<mp::std::vector<int>> triggerArrivals_;

        std::vector<ProtoCollision> const &protoCollisions_;
        std::vector<ProtoPortal> const &protoPortals_;
        //std::vector<ProtoButton> const &protoButtons_;

        SimpleConfiguredTriggerFrameState(SimpleConfiguredTriggerFrameState &o) = delete;
        SimpleConfiguredTriggerFrameState &operator=(SimpleConfiguredTriggerFrameState &o) = delete;
        SimpleConfiguredTriggerFrameState(SimpleConfiguredTriggerFrameState &&o) = delete;
        SimpleConfiguredTriggerFrameState &operator=(SimpleConfiguredTriggerFrameState &&o) = delete;
    };
    class SimpleConfiguredTriggerSystem final :
        public TriggerSystemImplementation
    {
        auto comparison_tuple() const -> decltype(auto)
        {
            assert(false);
            return std::tie();
            //Doesn't include luaStates_, since luaStates_ is just a cache.
            //return std::tie(compiledMainChunk_, compiledExtraChunks_, triggerOffsetsAndDefaults_, arrivalLocationsSize_);
        }
    public:
        SimpleConfiguredTriggerSystem(
            //std::vector<char> const &mainChunk,
            //std::vector<LuaModule> const &extraChunks,
            std::vector<
                std::pair<
                    int,
                    std::vector<int>
                >
            > triggerOffsetsAndDefaults,
            std::size_t arrivalLocationsSize);
        virtual TriggerFrameState getFrameState(memory_pool<user_allocator_tbb_alloc> &pool, OperationInterrupter &interrupter) const override;
        virtual TriggerSystemImplementation *clone() const override
        {
            return new SimpleConfiguredTriggerSystem(*this);
        }
        virtual bool operator==(TriggerSystemImplementation const &o) const override
        {
            SimpleConfiguredTriggerSystem const &actual_other(*boost::polymorphic_downcast<SimpleConfiguredTriggerSystem const*>(&o));
            return comparison_tuple() == actual_other.comparison_tuple();
        }
        virtual int order_ranking() const override
        {
            return 2000;
        }
    private:
        //lazy_ptr because TriggerSystemImplementations must
        //be cloneable, but there is no way to copy
        //a LuaState (or by extension a ThreadLocal<LuaState>).
        //copy_as_new_ptr side-steps this problem by making copying create
        //a fresh instance of the ThreadLocal<LuaState> every time.
        //luaStates_ a cache, so the act of ignoring its contents
        //does not cause any problems.
        //copy_as_new_ptr<ThreadLocal<LuaState>> luaStates_;

        //std::vector<char> compiledMainChunk_;
        //std::vector<LuaModule> compiledExtraChunks_;
        
        //std::vector<ProtoPortal> protoPortals;
        //std::vector<ProtoCollision> protoCollisions;
        //Rename Collision -> Collider?
        //std::vector<ProtoMutator> protoMutators;
        //std::vector<ProtoButton> protoButtons;
        
        std::vector<
            std::pair<
            int,
            std::vector<int>
            >
        > triggerOffsetsAndDefaults_;
        std::size_t arrivalLocationsSize_;
    };
}
#endif //HG_DIRECT_LUA_TRIGGER_SYSTEM_H
