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
    //TODO: Use DynamicArea in more places?
    struct DynamicArea {
        int x;
        int y;
        int xspeed;
        int yspeed;
        int width;
        int height;
        TimeDirection timeDirection;
    };

    struct AxisCollisionDestination final {
    private:
        auto comparison_tuple() const -> decltype(auto) {
            return std::tie(
                desiredPosition,
                acceleration,
                deceleration,
                maxSpeed
            );
        }
    public:
        int desiredPosition;
        int acceleration;
        int deceleration;
        int maxSpeed;
        bool operator==(AxisCollisionDestination const &o) const {
            return comparison_tuple() == o.comparison_tuple();
        }
    };
    struct CollisionDestination final {
    private:
        auto comparison_tuple() const -> decltype(auto) {
            return std::tie(
                xDestination,
                yDestination
            );
        }
    public:
        AxisCollisionDestination xDestination;
        AxisCollisionDestination yDestination;
        bool operator==(CollisionDestination const &o) const {
            return comparison_tuple() == o.comparison_tuple();
        }
    };
    struct ProtoCollision final {
    private:
        auto comparison_tuple() const -> decltype(auto) {
            return std::tie(
                timeDirection,
                width,
                height,
                onDestination,
                offDestination,
                hasButtonTriggerID,
                buttonTriggerID,
                lastStateTriggerID
            );
        }
    public:
        TimeDirection timeDirection;
        int width;
        int height;
        CollisionDestination onDestination;
        CollisionDestination offDestination;
        //TODO: More elegant handling of hasButtonTriggerID
        bool hasButtonTriggerID;
        int buttonTriggerID;
        int lastStateTriggerID;

        bool operator==(ProtoCollision const &o) const {
            return comparison_tuple() == o.comparison_tuple();
        }
    };
    struct Attachment final {
    private:
        auto comparison_tuple() const -> decltype(auto) {
            return std::tie(
                hasPlatform,
                platform,
                xOffset,
                yOffset
            );
        }
    public:

        bool hasPlatform;
        int platform;
        int xOffset;
        int yOffset;

        bool operator==(Attachment const &o) const{
            return comparison_tuple() == o.comparison_tuple();
        }
    };
    struct ProtoPortal final {
    private:
        auto comparison_tuple() const -> decltype(auto) {
            return std::tie(
                attachment,
                index,
                xaim,
                yaim,
                width,
                height,
                collisionOverlap,
                timeDirection,
                destinationIndex,
                xDestination,
                yDestination,
                relativeTime,
                timeDestination,
                relativeDirection,
                destinationDirection,
                illegalDestination,
                fallable,
                isLaser,
                winner
            );
        }
    public:

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

        bool operator==(ProtoPortal const &o) const {
            return comparison_tuple() == o.comparison_tuple();
        }
    };

    struct ButtonFrameStateImpl {
        virtual std::size_t clone_size() const = 0;
        virtual ButtonFrameStateImpl *perform_clone(void *memory) const = 0;
        virtual ~ButtonFrameStateImpl() {}

        virtual void calcPnV(mp::std::vector<Collision> const &collisions) = 0;
        virtual void updateState(
            mt::std::map<Frame*, ObjectList<Normal>> const &departures,
            mp::std::vector<mp::std::vector<int>> const &triggerArrivals) = 0;
        virtual void calculateGlitz(
            mt::std::vector<Glitz> &forwardsGlitz,
            mt::std::vector<Glitz> &reverseGlitz,
            mt::std::vector<GlitzPersister> &persistentGlitz) const = 0;
        virtual void fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const = 0;
    };
    
    struct ProtoMomentarySwitchImpl;

    struct MomentarySwitchFrameStateImpl final : ButtonFrameStateImpl {
        std::size_t clone_size() const override {
            return sizeof *this;
        }
        ButtonFrameStateImpl *perform_clone(void *memory) const override {
            return new (memory) MomentarySwitchFrameStateImpl(*this);
        }
        ~MomentarySwitchFrameStateImpl() override {}

        MomentarySwitchFrameStateImpl(ProtoMomentarySwitchImpl const &proto) :
            proto(&proto),
            justPressed(false),
            justReleased(false)
        {
        }


        void calcPnV(mp::std::vector<Collision> const &collisions) override;
        void updateState(
            mt::std::map<Frame*, ObjectList<Normal>> const &departures,
            mp::std::vector<mp::std::vector<int>> const &triggerArrivals) override;
        void calculateGlitz(
            mt::std::vector<Glitz> &forwardsGlitz,
            mt::std::vector<Glitz> &reverseGlitz,
            mt::std::vector<GlitzPersister> &persistentGlitz) const override;
        void fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const override;
    private:
        ProtoMomentarySwitchImpl const *proto;
        int state;
        bool justPressed;
        bool justReleased;

        int x;
        int y;
        int xspeed;
        int yspeed;
    };

    struct ProtoStickySwitchImpl;

    struct StickySwitchFrameStateImpl final : ButtonFrameStateImpl {
        std::size_t clone_size() const override {
            return sizeof *this;
        }
        ButtonFrameStateImpl *perform_clone(void *memory) const override {
            return new (memory) StickySwitchFrameStateImpl(*this);
        }
        ~StickySwitchFrameStateImpl() override {}

        StickySwitchFrameStateImpl(ProtoStickySwitchImpl const &proto) :
            proto(&proto),
            justPressed(false)
        {
        }

        void calcPnV(mp::std::vector<Collision> const &collisions) override;
        void updateState(
            mt::std::map<Frame*, ObjectList<Normal>> const &departures,
            mp::std::vector<mp::std::vector<int>> const &triggerArrivals) override;
        void fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const override;
        void calculateGlitz(
            mt::std::vector<Glitz> &forwardsGlitz,
            mt::std::vector<Glitz> &reverseGlitz,
            mt::std::vector<GlitzPersister> &persistentGlitz) const override;
    private:
        ProtoStickySwitchImpl const *proto;
        int state;
        bool justPressed;

        int x;
        int y;
        int xspeed;
        int yspeed;
    };


    struct ButtonFrameState final {
        template<typename ButtonFrameStateImpl>
        ButtonFrameState(ButtonFrameStateImpl *impl, hg::memory_pool<hg::user_allocator_tbb_alloc> &pool)
            : pimpl_(impl, memory_source_clone<hg::ButtonFrameStateImpl, memory_pool_memory_source>(memory_pool_memory_source(pool)))
        {}

        void calcPnV(mp::std::vector<Collision> const &collisions) {
            assert(pimpl_);
            pimpl_->calcPnV(collisions);
        }
        void updateState(
            mt::std::map<Frame*, ObjectList<Normal>> const &departures,
            mp::std::vector<mp::std::vector<int>> const &triggerArrivals)
        {
            assert(pimpl_);
            pimpl_->updateState(departures, triggerArrivals);
        }
        void fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const/*?*/{
            assert(pimpl_);
            pimpl_->fillTrigger(outputTriggers);
        }
        void calculateGlitz(
            mt::std::vector<Glitz> &forwardsGlitz,
            mt::std::vector<Glitz> &reverseGlitz,
            mt::std::vector<GlitzPersister> &persistentGlitz) const
        {
            assert(pimpl_);
            pimpl_->calculateGlitz(forwardsGlitz, reverseGlitz, persistentGlitz);
        }
    private:
        clone_ptr<ButtonFrameStateImpl, memory_source_clone<ButtonFrameStateImpl, memory_pool_memory_source>> pimpl_;
    };


    struct ProtoButtonImpl {
        virtual std::size_t clone_size() const = 0;
        virtual ProtoButtonImpl *perform_clone(void *memory) const = 0;
        virtual ~ProtoButtonImpl() {}
        virtual ButtonFrameState getFrameState(hg::memory_pool<hg::user_allocator_tbb_alloc> & pool) const = 0;

        virtual int order_ranking() const = 0;
        virtual bool operator==(ProtoButtonImpl const &o) const = 0;
    };

    struct ProtoMomentarySwitchImpl final : ProtoButtonImpl {
    private:
        auto comparison_tuple() const -> decltype(auto) {
            return std::tie(
                timeDirection,
                attachment,
                width,
                height,
                triggerID,
                stateTriggerID,
                extraTriggerIDs
            );
        }
    public:

        std::size_t clone_size() const override {
            return sizeof *this;
        }
        ProtoButtonImpl *perform_clone(void *memory) const override {
            return new (memory) ProtoMomentarySwitchImpl(*this);
        }
        ~ProtoMomentarySwitchImpl() override {}
        ButtonFrameState getFrameState(hg::memory_pool<hg::user_allocator_tbb_alloc> & pool) const override {
            return ButtonFrameState(new (pool) MomentarySwitchFrameStateImpl(*this), pool);
        }

        ProtoMomentarySwitchImpl(
            TimeDirection const timeDirection,
            Attachment const attachment,
            int const width,
            int const height,
            int const triggerID,
            int const stateTriggerID,
            std::vector<int> extraTriggerIDs
        ) :
            timeDirection(timeDirection),
            attachment(attachment),
            width(width),
            height(height),
            triggerID(triggerID),
            stateTriggerID(stateTriggerID),
            extraTriggerIDs(std::move(extraTriggerIDs))
        {}

        TimeDirection timeDirection;
        Attachment attachment;
        int width;
        int height;
        int triggerID;
        int stateTriggerID;
        std::vector<int> extraTriggerIDs;

        virtual int order_ranking() const override {
            return 1000;
        }
        virtual bool operator==(ProtoButtonImpl const &o) const override {
            ProtoMomentarySwitchImpl const &actual_other(*boost::polymorphic_downcast<ProtoMomentarySwitchImpl const*>(&o));
            return comparison_tuple() == actual_other.comparison_tuple();
        }
    };


    struct ProtoStickySwitchImpl final : ProtoButtonImpl {
    private:
        auto comparison_tuple() const -> decltype(auto) {
            return std::tie(
                timeDirection,
                attachment,
                width,
                height,
                triggerID,
                stateTriggerID,
                extraTriggerIDs
            );
        }
    public:
        std::size_t clone_size() const override {
            return sizeof *this;
        }
        ProtoButtonImpl *perform_clone(void *memory) const override {
            return new (memory) ProtoStickySwitchImpl(*this);
        }
        ~ProtoStickySwitchImpl() override {}
        ButtonFrameState getFrameState(hg::memory_pool<hg::user_allocator_tbb_alloc> & pool) const override {
            return ButtonFrameState(new (pool) StickySwitchFrameStateImpl(*this), pool);
        }

        ProtoStickySwitchImpl(
            TimeDirection const timeDirection,
            Attachment const attachment,
            int const width,
            int const height,
            int const triggerID,
            int const stateTriggerID,
            std::vector<int> extraTriggerIDs
        ) :
            timeDirection(timeDirection),
            attachment(attachment),
            width(width),
            height(height),
            triggerID(triggerID),
            stateTriggerID(stateTriggerID),
            extraTriggerIDs(std::move(extraTriggerIDs))
        {}

        TimeDirection timeDirection;
        Attachment attachment;
        int width;
        int height;
        int triggerID;
        int stateTriggerID;
        std::vector<int> extraTriggerIDs;

        virtual int order_ranking() const override {
            return 2000;
        }
        virtual bool operator==(ProtoButtonImpl const &o) const override {
            ProtoStickySwitchImpl const &actual_other(*boost::polymorphic_downcast<ProtoStickySwitchImpl const*>(&o));
            return comparison_tuple() == actual_other.comparison_tuple();
        }
    };


    struct ProtoButton final {
    private:
        clone_ptr<ProtoButtonImpl, memory_source_clone<ProtoButtonImpl, multi_thread_memory_source>> pimpl_;
        typedef
            std::tuple<
                decltype(pimpl_->order_ranking()),
                ProtoButtonImpl const &>
            comparison_tuple_type;
        comparison_tuple_type comparison_tuple() const {
            return comparison_tuple_type(pimpl_->order_ranking(), *pimpl_);
        }
    public:
        
        explicit ProtoButton(mt::std::unique_ptr<ProtoButtonImpl> impl)
            : pimpl_(impl.release())
        {
            assert(pimpl_);
        }
        ButtonFrameState getFrameState(hg::memory_pool<hg::user_allocator_tbb_alloc> & pool) const {
            assert(pimpl_);
            return pimpl_->getFrameState(pool);
        }
        bool operator==(ProtoButton const &o) const {
            return comparison_tuple() == o.comparison_tuple();
        }
    };

    struct MutatorFrameState;

    struct MutatorFrameStateImpl {
        virtual std::size_t clone_size() const = 0;
        virtual MutatorFrameStateImpl *perform_clone(void *memory) const = 0;
        virtual ~MutatorFrameStateImpl() {}

        virtual void addMutator(
            mp::std::vector<mp::std::vector<int>> const &triggerArrivals,
            mp::std::vector<Collision> const &collisions,
            mp::std::vector<MutatorArea> &mutators,
            mp::std::vector<MutatorFrameStateImpl *> &activeMutators
        ) = 0;
        virtual void calculateGlitz(
            mt::std::vector<Glitz> &forwardsGlitz,
            mt::std::vector<Glitz> &reverseGlitz,
            mt::std::vector<GlitzPersister> &persistentGlitz) const = 0;
        virtual void fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const = 0;
        virtual boost::optional<Guy> effect(Guy const &guy) = 0;
        virtual boost::optional<Box> effect(Box const &box) = 0;
    };

    struct ProtoPickupImpl;

    struct PickupFrameStateImpl final : MutatorFrameStateImpl {
    private:
        ProtoPickupImpl const *proto;
        bool active;
        bool justTaken;
        int x_;
        int y_;
    public:
        PickupFrameStateImpl(ProtoPickupImpl const &proto):
            proto(&proto),
            active(true),
            justTaken(false)
        {}
        std::size_t clone_size() const override {
            return sizeof *this;
        }
        MutatorFrameStateImpl *perform_clone(void *memory) const override {
            return new (memory) PickupFrameStateImpl(*this);
        }
        ~PickupFrameStateImpl() override {}

        void addMutator(
            mp::std::vector<mp::std::vector<int>> const &triggerArrivals,
            mp::std::vector<Collision> const &collisions,
            mp::std::vector<MutatorArea> &mutators,
            mp::std::vector<MutatorFrameStateImpl *> &activeMutators
        ) override;

        void calculateGlitz(
            mt::std::vector<Glitz> &forwardsGlitz,
            mt::std::vector<Glitz> &reverseGlitz,
            mt::std::vector<GlitzPersister> &persistentGlitz) const override;
        void fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const override;
        boost::optional<Guy> effect(Guy const &guy) override;
        boost::optional<Box> effect(Box const &box) override {
            return box;
        }
    };

    struct ProtoSpikesImpl;

    struct SpikesFrameStateImpl final : MutatorFrameStateImpl {
    private:
        ProtoSpikesImpl const *proto;
        mp::std::vector<DynamicArea> deathGlitz;
        int x_;
        int y_;
        int xspeed_;
        int yspeed_;

    public:
        SpikesFrameStateImpl(ProtoSpikesImpl const &proto, hg::memory_pool<hg::user_allocator_tbb_alloc> &pool):
            proto(&proto),
            deathGlitz(pool),
            x_(),
            y_(),
            xspeed_(),
            yspeed_()
        {}
        std::size_t clone_size() const override {
            return sizeof *this;
        }
        MutatorFrameStateImpl *perform_clone(void *memory) const override {
            return new (memory) SpikesFrameStateImpl(*this);
        }
        ~SpikesFrameStateImpl() override {}

        void addMutator(
            mp::std::vector<mp::std::vector<int>> const &triggerArrivals,
            mp::std::vector<Collision> const &collisions,
            mp::std::vector<MutatorArea> &mutators,
            mp::std::vector<MutatorFrameStateImpl *> &activeMutators
        ) override;

        void calculateGlitz(
            mt::std::vector<Glitz> &forwardsGlitz,
            mt::std::vector<Glitz> &reverseGlitz,
            mt::std::vector<GlitzPersister> &persistentGlitz) const override;
        void fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const override;
        boost::optional<Guy> effect(Guy const &guy) override;
        boost::optional<Box> effect(Box const &box) override {
            return box;
        }
    };

    struct MutatorFrameState final {
        MutatorFrameState(MutatorFrameStateImpl *const impl, hg::memory_pool<hg::user_allocator_tbb_alloc> &pool) :
            pimpl_(
                impl,
                memory_source_clone<MutatorFrameStateImpl, memory_pool_memory_source>(memory_pool_memory_source(pool)))
        {}
        void addMutator(
            mp::std::vector<mp::std::vector<int>> const &triggerArrivals,
            mp::std::vector<Collision> const &collisions,
            mp::std::vector<MutatorArea> &mutators,
            mp::std::vector<MutatorFrameStateImpl *> &activeMutators
        ) const
        {
            assert(pimpl_);
            return pimpl_->addMutator(triggerArrivals, collisions, mutators, activeMutators);
        }

        void fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const/*?*/{
            assert(pimpl_);
            pimpl_->fillTrigger(outputTriggers);
        }
        void calculateGlitz(
            mt::std::vector<Glitz> &forwardsGlitz,
            mt::std::vector<Glitz> &reverseGlitz,
            mt::std::vector<GlitzPersister> &persistentGlitz) const
        {
            assert(false);
            assert(pimpl_);
            pimpl_->calculateGlitz(forwardsGlitz, reverseGlitz, persistentGlitz);
        }
        boost::optional<Guy> effect(Guy const &guy) {
            assert(pimpl_);
            return pimpl_->effect(guy);
        }
        boost::optional<Box> effect(Box const &box) {
            assert(pimpl_);
            return pimpl_->effect(box);
        }

    private:
        clone_ptr<MutatorFrameStateImpl, memory_source_clone<MutatorFrameStateImpl, memory_pool_memory_source>> pimpl_;
    };

    struct ProtoMutatorImpl {
        virtual MutatorFrameState getFrameState(hg::memory_pool<hg::user_allocator_tbb_alloc> & pool) const = 0;
        virtual std::size_t clone_size() const = 0;
        virtual ProtoMutatorImpl *perform_clone(void *memory) const = 0;
        virtual ~ProtoMutatorImpl() {}
        virtual int order_ranking() const = 0;
        virtual bool operator==(ProtoMutatorImpl const &o) const = 0;
    };
    struct ProtoPickupImpl final : ProtoMutatorImpl {
    private:
        auto comparison_tuple() const -> decltype(auto) {
            return std::tie(
                timeDirection,
                attachment,
                width,
                height,
                pickupType,
                pickupNumber,
                triggerID
            );
        }
    public:

        MutatorFrameState getFrameState(hg::memory_pool<hg::user_allocator_tbb_alloc> & pool) const override {
            return MutatorFrameState(new (pool) PickupFrameStateImpl(*this), pool);
        }
        std::size_t clone_size() const override {
            return sizeof *this;
        }
        ProtoMutatorImpl *perform_clone(void *memory) const override {
            return new (memory) ProtoPickupImpl(*this);
        }

        ~ProtoPickupImpl() override {}
        ProtoPickupImpl(
            TimeDirection const timeDirection,
            Attachment const &attachment,
            int const width,
            int const height,
            Ability const pickupType,
            int const pickupNumber,
            int const triggerID
        ) :
            timeDirection(timeDirection),
            attachment(attachment),
            width(width),
            height(height),
            pickupType(pickupType),
            pickupNumber(pickupNumber),
            triggerID(triggerID)
        {}

        TimeDirection timeDirection;
        Attachment attachment;
        int width;
        int height;
        Ability pickupType;
        int pickupNumber;
        int triggerID;

        virtual int order_ranking() const override {
            return 1000;
        }
        virtual bool operator==(ProtoMutatorImpl const &o) const override {
            ProtoPickupImpl const &actual_other(*boost::polymorphic_downcast<ProtoPickupImpl const*>(&o));
            return comparison_tuple() == actual_other.comparison_tuple();
        }
    };


    struct ProtoSpikesImpl final : ProtoMutatorImpl {
    private:
        auto comparison_tuple() const -> decltype(auto) {
            return std::tie(
                timeDirection,
                attachment,
                width,
                height
            );
        }
    public:
        MutatorFrameState getFrameState(hg::memory_pool<hg::user_allocator_tbb_alloc> &pool) const override {
            return MutatorFrameState(new (pool) SpikesFrameStateImpl(*this, pool), pool);
        }
        std::size_t clone_size() const override {
            return sizeof *this;
        }
        ProtoMutatorImpl *perform_clone(void *memory) const override {
            return new (memory) ProtoSpikesImpl(*this);
        }

        ~ProtoSpikesImpl() override {}
        ProtoSpikesImpl(
            TimeDirection const timeDirection,
            Attachment const &attachment,
            int const width,
            int const height
        ) :
            timeDirection(timeDirection),
            attachment(attachment),
            width(width),
            height(height)
        {}

        TimeDirection timeDirection;
        Attachment attachment;
        int width;
        int height;

        virtual int order_ranking() const override {
            return 2000;
        }
        virtual bool operator==(ProtoMutatorImpl const &o) const override {
            ProtoSpikesImpl const &actual_other(*boost::polymorphic_downcast<ProtoSpikesImpl const*>(&o));
            return comparison_tuple() == actual_other.comparison_tuple();
        }
    };



    struct ProtoMutator final {
    private:
        clone_ptr<ProtoMutatorImpl, memory_source_clone<ProtoMutatorImpl, multi_thread_memory_source>> pimpl_;
        typedef
            std::tuple<
                decltype(pimpl_->order_ranking()),
                ProtoMutatorImpl const &>
            comparison_tuple_type;
        comparison_tuple_type comparison_tuple() const {
            return comparison_tuple_type(pimpl_->order_ranking(), *pimpl_);
        }
    public:
        explicit ProtoMutator(mt::std::unique_ptr<ProtoMutatorImpl> impl)
            : pimpl_(impl.release())
        {
            assert(pimpl_);
        }

        MutatorFrameState getFrameState(hg::memory_pool<hg::user_allocator_tbb_alloc> &pool) const {
            assert(pimpl_);
            return pimpl_->getFrameState(pool);
        }

        bool operator==(ProtoMutator const &o) const {
            return comparison_tuple() == o.comparison_tuple();
        }
    };
    
    class SimpleConfiguredTriggerFrameState final :
        public TriggerFrameStateImplementation
    {
    public:
        SimpleConfiguredTriggerFrameState(
            std::vector<
                std::pair<int, std::vector<int>>
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
        mp::std::vector<MutatorFrameStateImpl *> activeMutators_;
        mp::std::vector<mp::std::vector<int>> triggerArrivals_;

        std::vector<ProtoCollision> const &protoCollisions_;
        std::vector<ProtoPortal> const &protoPortals_;

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
            return std::tie(
                protoPortals_,
                protoCollisions_,
                protoMutators_,
                protoButtons_,
                triggerOffsetsAndDefaults_,
                arrivalLocationsSize_);
        }
    public:
        SimpleConfiguredTriggerSystem(
            std::vector<char> const &mainChunk,
            std::vector<LuaModule> const &extraChunks,
            std::vector<
                std::pair<
                    int,
                    std::vector<int>
                >
            > triggerOffsetsAndDefaults,
            std::size_t arrivalLocationsSize);
        virtual TriggerFrameState getFrameState(memory_pool<user_allocator_tbb_alloc> &pool, OperationInterrupter &interrupter) const override{
            return TriggerFrameState(
                new (pool) SimpleConfiguredTriggerFrameState(
                    triggerOffsetsAndDefaults_,
                    arrivalLocationsSize_,
                    protoCollisions_,
                    protoPortals_,
                    protoButtons_,
                    protoMutators_,
                    pool,
                    interrupter));
        }
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

        std::vector<ProtoPortal> protoPortals_;
        std::vector<ProtoCollision> protoCollisions_;
        //Rename Collision -> Collider?
        //TODO: Use boost polymorphic collection rather than vector of clone_ptr?
        std::vector<ProtoMutator> protoMutators_;
        std::vector<ProtoButton> protoButtons_;

        std::vector<
            std::pair<int, std::vector<int>>
        > triggerOffsetsAndDefaults_;
        std::size_t arrivalLocationsSize_;
    };

}
#endif //HG_SIMPLE_CONFIGURED_TRIGGER_SYSTEM_H
