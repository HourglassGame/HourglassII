#ifndef HG_SIMPLE_CONFIGURED_TRIGGER_SYSTEM_H
#define HG_SIMPLE_CONFIGURED_TRIGGER_SYSTEM_H
#include "TriggerSystemImplementation.h"
#include "copy_as_new_ptr.h"
#include "SimpleLuaCpp.h"
#include "TextGlitz.h"
#include "ThreadLocal.h"
#include "ObjectAndTime.h"
#include "LuaInterruption.h"
#include "LuaModule.h"
#include "memory_pool.h"
#include "Powerup.h"
#include "TriggerClause.h"
#include <string>
#include <vector>
#include <mutex>
#include "mp/std/map"
#include <optional>
#include <boost/polymorphic_cast.hpp>
#include <tuple>
namespace hg {
    class OperationInterrupter;
    struct PositionAndVelocity2D final {
        int x;
        int y;
        int xspeed;
        int yspeed;
    };
    //TODO: Use DynamicArea in more places?
    struct DynamicArea final {
        int x;
        int y;
        int xspeed;
        int yspeed;
        int width;
        int height;
        TimeDirection timeDirection;
    };
    std::tuple<Glitz, Glitz> calculateBidirectionalGlitz(
        int const layer,
        DynamicArea const &dynamicArea,
        unsigned const forwardsColour,
        unsigned const reverseColour);
    unsigned asPackedColour(int const r, int const g, int const b);
    struct AxisCollisionDestination final {
    private:
        auto comparison_tuple() const noexcept -> decltype(auto) {
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
        bool operator==(AxisCollisionDestination const &o) const noexcept {
            return comparison_tuple() == o.comparison_tuple();
        }
    };
    struct CollisionDestination final {
    private:
        auto comparison_tuple() const noexcept -> decltype(auto) {
            return std::tie(
                xDestination,
                yDestination
            );
        }
    public:
        AxisCollisionDestination xDestination;
        AxisCollisionDestination yDestination;
        bool operator==(CollisionDestination const &o) const noexcept {
            return comparison_tuple() == o.comparison_tuple();
        }
    };
    struct ProtoCollision final {
    private:
        auto comparison_tuple() const noexcept -> decltype(auto) {
            return std::tie(
                timeDirection,
                width,
                height,
                onDestination,
                offDestination,
                hasButtonTriggerID,
                buttonTriggerID,
                //hasTriggerClause,
                //triggerClause,
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
        bool hasTriggerClause;
        TriggerClause triggerClause;
        int lastStateTriggerID;

        bool operator==(ProtoCollision const &o) const noexcept {
            return comparison_tuple() == o.comparison_tuple();
        }
    };
    struct Attachment final {
    private:
        auto comparison_tuple() const noexcept -> decltype(auto) {
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

        bool operator==(Attachment const &o) const noexcept {
            return comparison_tuple() == o.comparison_tuple();
        }
    };
    struct ButtonSegment final {
    private:
        auto comparison_tuple() const {
            return std::tie(attachment, width, height);
        }
    public:
        Attachment attachment;
        int width;
        int height;
        bool operator==(ButtonSegment const &o) const noexcept {
            return comparison_tuple() == o.comparison_tuple();
        }
    };
    struct ProtoPortal final {
    private:
        auto comparison_tuple() const noexcept -> decltype(auto) {
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
                //hasTriggerClause,
                //triggerClause,
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
        bool hasTriggerClause;
        TriggerClause triggerClause;
        bool fallable;
        bool isLaser;
        bool winner;

        bool operator==(ProtoPortal const &o) const noexcept {
            return comparison_tuple() == o.comparison_tuple();
        }
    };

    struct ProtoTriggerMod final {
    private:
        auto comparison_tuple() const noexcept -> decltype(auto) {
            return std::tie(
                triggerID,
                //triggerClause
                useTriggerArrival
            );
        }

    public:

        int const triggerID;
        TriggerClause const triggerClause;
        bool const useTriggerArrival;

        void modifyTrigger(
            mp::std::vector<mp::std::vector<int>> const &triggerArrivals,
            mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers,
            Frame const *const currentFrame) const
        {
            if (useTriggerArrival) {
                outputTriggers[triggerID] = mt::std::vector<int>{ triggerClause.GetOutput(triggerArrivals, getFrameNumber(currentFrame)) };
            }
            else
            {
                outputTriggers[triggerID] = mt::std::vector<int>{ triggerClause.GetOutput(outputTriggers, getFrameNumber(currentFrame)) };
            }
        }
        bool operator==(ProtoTriggerMod const &o) const noexcept {
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
        std::size_t clone_size() const noexcept final {
            return sizeof *this;
        }
        ButtonFrameStateImpl *perform_clone(void *memory) const final {
            return new (memory) MomentarySwitchFrameStateImpl(*this);
        }
        ~MomentarySwitchFrameStateImpl() noexcept final = default;

        MomentarySwitchFrameStateImpl(ProtoMomentarySwitchImpl const &proto) :
            proto(&proto),
            justPressed(false),
            justReleased(false)
        {
        }


        void calcPnV(mp::std::vector<Collision> const &collisions) final;
        void updateState(
            mt::std::map<Frame*, ObjectList<Normal>> const &departures,
            mp::std::vector<mp::std::vector<int>> const &triggerArrivals) final;
        void calculateGlitz(
            mt::std::vector<Glitz> &forwardsGlitz,
            mt::std::vector<Glitz> &reverseGlitz,
            mt::std::vector<GlitzPersister> &persistentGlitz) const final;
        void fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const final;
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
        std::size_t clone_size() const noexcept final {
            return sizeof *this;
        }
        ButtonFrameStateImpl *perform_clone(void *memory) const final {
            return new (memory) StickySwitchFrameStateImpl(*this);
        }
        ~StickySwitchFrameStateImpl() noexcept final = default;

        StickySwitchFrameStateImpl(ProtoStickySwitchImpl const &proto) :
            proto(&proto),
            justPressed(false)
        {
        }

        void calcPnV(mp::std::vector<Collision> const &collisions) final;
        void updateState(
            mt::std::map<Frame*, ObjectList<Normal>> const &departures,
            mp::std::vector<mp::std::vector<int>> const &triggerArrivals) final;
        void fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const final;
        void calculateGlitz(
            mt::std::vector<Glitz> &forwardsGlitz,
            mt::std::vector<Glitz> &reverseGlitz,
            mt::std::vector<GlitzPersister> &persistentGlitz) const final;
    private:
        ProtoStickySwitchImpl const *proto;
        int state;
        bool justPressed;

        int x;
        int y;
        int xspeed;
        int yspeed;
    };

    struct ProtoToggleSwitchImpl;

    struct ToggleSwitchFrameStateImpl final : ButtonFrameStateImpl {
        std::size_t clone_size() const noexcept final {
            return sizeof *this;
        }
        ButtonFrameStateImpl *perform_clone(void *memory) const final {
            return new (memory) ToggleSwitchFrameStateImpl(*this);
        }
        ~ToggleSwitchFrameStateImpl() noexcept final = default;

        ToggleSwitchFrameStateImpl(ProtoToggleSwitchImpl const &proto) :
            proto(&proto)
        {
        }

        void calcPnV(mp::std::vector<Collision> const &collisions) final;
        void updateState(
            mt::std::map<Frame*, ObjectList<Normal>> const &departures,
            mp::std::vector<mp::std::vector<int>> const &triggerArrivals) final;
        void fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const final;
        void calculateGlitz(
            mt::std::vector<Glitz> &forwardsGlitz,
            mt::std::vector<Glitz> &reverseGlitz,
            mt::std::vector<GlitzPersister> &persistentGlitz) const final;
    private:
        ProtoToggleSwitchImpl const *proto;//Initialised by ctor
        PositionAndVelocity2D firstPnV;//Initialised by calcPnV
        PositionAndVelocity2D secondPnV;//Initialised by calcPnV
        bool switchState;//Initialised by updateState
        bool justPressed;//Initialised by updateState
    };


    struct ProtoMultiStickySwitchImpl;

    struct MultiStickySwitchFrameStateImpl final : ButtonFrameStateImpl {
        std::size_t clone_size() const noexcept final {
            return sizeof *this;
        }
        ButtonFrameStateImpl *perform_clone(void *memory) const final {
            return new (memory) MultiStickySwitchFrameStateImpl(*this);
        }
        ~MultiStickySwitchFrameStateImpl() noexcept final = default;

        MultiStickySwitchFrameStateImpl(
            ProtoMultiStickySwitchImpl const &proto,
            hg::memory_pool<hg::user_allocator_tbb_alloc> &pool);

        void calcPnV(mp::std::vector<Collision> const &collisions) final;
        void updateState(
            mt::std::map<Frame*, ObjectList<Normal>> const &departures,
            mp::std::vector<mp::std::vector<int>> const &triggerArrivals) final;
        void fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const final;
        void calculateGlitz(
            mt::std::vector<Glitz> &forwardsGlitz,
            mt::std::vector<Glitz> &reverseGlitz,
            mt::std::vector<GlitzPersister> &persistentGlitz) const final;
    private:
        ProtoMultiStickySwitchImpl const *proto;//Initialised by ctor
        mp::std::vector<PositionAndVelocity2D> PnVs;//Initialised by ctor/calcPnV
        
        int switchState;//Initialised by updateState
        mp::std::vector<char> individualState;//Initialised by ctor/updateState
        mp::std::vector<char> justPressed;//Initialised by ctor/updateState
        mp::std::vector<char> justReleased;//Initialised by ctor/updateState
    };

    struct ProtoStickyLaserSwitchImpl;

    struct StickyLaserSwitchFrameStateImpl final : ButtonFrameStateImpl {
        std::size_t clone_size() const noexcept final {
            return sizeof *this;
        }
        ButtonFrameStateImpl *perform_clone(void *memory) const final {
            return new (memory) StickyLaserSwitchFrameStateImpl(*this);
        }
        ~StickyLaserSwitchFrameStateImpl() noexcept final = default;

        StickyLaserSwitchFrameStateImpl(ProtoStickyLaserSwitchImpl const &proto)
            : proto(&proto)
        {}

        void calcPnV(mp::std::vector<Collision> const &collisions) final;
        void updateState(
            mt::std::map<Frame*, ObjectList<Normal>> const &departures,
            mp::std::vector<mp::std::vector<int>> const &triggerArrivals) final;
        void fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const final;
        void calculateGlitz(
            mt::std::vector<Glitz> &forwardsGlitz,
            mt::std::vector<Glitz> &reverseGlitz,
            mt::std::vector<GlitzPersister> &persistentGlitz) const final;
    private:
        ProtoStickyLaserSwitchImpl const *proto;//Initialised by ctor
        PositionAndVelocity2D beamPnV; //Initialised by calcPnV
        PositionAndVelocity2D emitterPnV; //Initialised by calcPnV

        char switchState;//Initialised by updateState
        bool justPressed;//Initialised by updateState
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
        virtual ~ProtoButtonImpl() noexcept = default;
        virtual ButtonFrameState getFrameState(hg::memory_pool<hg::user_allocator_tbb_alloc> & pool) const = 0;

        /*
         1000 ProtoMomentarySwitchImpl
         2000 ProtoStickySwitchImpl
         3000 ProtoToggleSwitchImpl
         4000 ProtoMultiStickySwitchImpl
         5000 ProtoStickyLaserSwitchImpl
        */
        virtual int order_ranking() const = 0;
        virtual bool operator==(ProtoButtonImpl const &o) const = 0;
    };

    struct ProtoMomentarySwitchImpl final : ProtoButtonImpl {
    private:
        auto comparison_tuple() const noexcept {
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

        std::size_t clone_size() const noexcept final {
            return sizeof *this;
        }
        ProtoButtonImpl *perform_clone(void *memory) const final {
            return new (memory) ProtoMomentarySwitchImpl(*this);
        }
        ~ProtoMomentarySwitchImpl() noexcept final = default;
        ButtonFrameState getFrameState(hg::memory_pool<hg::user_allocator_tbb_alloc> & pool) const final {
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

        int order_ranking() const noexcept final {
            return 1000;
        }
        bool operator==(ProtoButtonImpl const &o) const noexcept final {
            ProtoMomentarySwitchImpl const &actual_other(*boost::polymorphic_downcast<ProtoMomentarySwitchImpl const*>(&o));
            return comparison_tuple() == actual_other.comparison_tuple();
        }
    };


    struct ProtoStickySwitchImpl final : ProtoButtonImpl {
    private:
        auto comparison_tuple() const noexcept -> decltype(auto) {
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
        std::size_t clone_size() const noexcept final {
            return sizeof *this;
        }
        ProtoButtonImpl *perform_clone(void *memory) const final {
            return new (memory) ProtoStickySwitchImpl(*this);
        }
        ~ProtoStickySwitchImpl() noexcept final = default;
        ButtonFrameState getFrameState(hg::memory_pool<hg::user_allocator_tbb_alloc> &pool) const final {
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

        int order_ranking() const noexcept final {
            return 2000;
        }
        bool operator==(ProtoButtonImpl const &o) const noexcept final {
            ProtoStickySwitchImpl const &actual_other(*boost::polymorphic_downcast<ProtoStickySwitchImpl const*>(&o));
            return comparison_tuple() == actual_other.comparison_tuple();
        }
    };


    struct ProtoToggleSwitchImpl final : ProtoButtonImpl {
    private:
        auto comparison_tuple() const noexcept {
            return std::tie(
                timeDirection,
                first,
                second,
                triggerID,
                stateTriggerID,
                extraTriggerIDs
            );
        }
        
    public:
        std::size_t clone_size() const noexcept final {
            return sizeof *this;
        }
        ProtoButtonImpl *perform_clone(void *memory) const final {
            return new (memory) ProtoToggleSwitchImpl(*this);
        }
        ~ProtoToggleSwitchImpl() noexcept final = default;
        ButtonFrameState getFrameState(hg::memory_pool<hg::user_allocator_tbb_alloc> &pool) const final {
            return ButtonFrameState(new (pool) ToggleSwitchFrameStateImpl(*this), pool);
        }

        ProtoToggleSwitchImpl(
            TimeDirection const timeDirection,
            ButtonSegment const &first,
            ButtonSegment const &second,
            int const triggerID,
            int const stateTriggerID,
            std::vector<int> extraTriggerIDs
        ) :
            timeDirection(timeDirection),
            first(first),
            second(second),
            triggerID(triggerID),
            stateTriggerID(stateTriggerID),
            extraTriggerIDs(std::move(extraTriggerIDs))
        {}
        
        TimeDirection timeDirection;
        ButtonSegment first;
        ButtonSegment second;
        int triggerID;
        int stateTriggerID;
        std::vector<int> extraTriggerIDs;

        int order_ranking() const noexcept final {
            return 3000;
        }
        
        bool operator==(ProtoButtonImpl const &o) const noexcept final {
            auto const &actual_other(*boost::polymorphic_downcast<ProtoToggleSwitchImpl const*>(&o));
            return comparison_tuple() == actual_other.comparison_tuple();
        }
    };

    struct ProtoMultiStickySwitchImpl final : ProtoButtonImpl {
    private:
        auto comparison_tuple() const noexcept {
            return std::tie(
                timeDirection,
                buttons,
                triggerID,
                stateTriggerID,
                extraTriggerIDs
            );
        }
        
    public:
        std::size_t clone_size() const noexcept final {
            return sizeof *this;
        }
        ProtoButtonImpl *perform_clone(void *memory) const final {
            return new (memory) ProtoMultiStickySwitchImpl(*this);
        }
        ~ProtoMultiStickySwitchImpl() noexcept final = default;
        ButtonFrameState getFrameState(hg::memory_pool<hg::user_allocator_tbb_alloc> &pool) const final {
            return ButtonFrameState(new (pool) MultiStickySwitchFrameStateImpl(*this, pool), pool);
        }

        ProtoMultiStickySwitchImpl(
            TimeDirection const timeDirection,
            std::vector<ButtonSegment> buttons,
            int const triggerID,
            int const stateTriggerID,
            std::vector<int> extraTriggerIDs
        ) :
            timeDirection(timeDirection),
            buttons(std::move(buttons)),
            triggerID(triggerID),
            stateTriggerID(stateTriggerID),
            extraTriggerIDs(std::move(extraTriggerIDs))
        {}
        
        TimeDirection timeDirection;
        std::vector<ButtonSegment> buttons;
        int triggerID;
        int stateTriggerID;
        std::vector<int> extraTriggerIDs;

        int order_ranking() const noexcept final {
            return 4000;
        }
        
        bool operator==(ProtoButtonImpl const &o) const noexcept final {
            auto const &actual_other(*boost::polymorphic_downcast<ProtoMultiStickySwitchImpl const*>(&o));
            return comparison_tuple() == actual_other.comparison_tuple();
        }
    };

    struct ProtoStickyLaserSwitchImpl final : ProtoButtonImpl {
    private:
        auto comparison_tuple() const noexcept {
            return std::tie(
                timeDirection,
                beam,
                emitter,
                triggerID,
                stateTriggerID,
                extraTriggerIDs
            );
        }
        
    public:
        std::size_t clone_size() const noexcept final {
            return sizeof *this;
        }
        ProtoButtonImpl *perform_clone(void *memory) const final {
            return new (memory) ProtoStickyLaserSwitchImpl(*this);
        }
        ~ProtoStickyLaserSwitchImpl() noexcept final = default;
        ButtonFrameState getFrameState(hg::memory_pool<hg::user_allocator_tbb_alloc> &pool) const final {
            return ButtonFrameState(new (pool) StickyLaserSwitchFrameStateImpl(*this), pool);
        }

        ProtoStickyLaserSwitchImpl(
            TimeDirection const timeDirection,
            Attachment const attachment,
            int const beamLength,
            int const beamDirection,
            int const triggerID,
            int const stateTriggerID,
            std::vector<int> extraTriggerIDs
        ) :
            timeDirection(timeDirection),
            triggerID(triggerID),
            stateTriggerID(stateTriggerID),
            extraTriggerIDs(std::move(extraTriggerIDs))
        {
            //beamDirection:
            //0 Right
            //1 Down
            //2 Left
            //3 Up

            int const beamWidth = 120;

            bool const vertical = beamDirection == 1 || beamDirection == 3;
            bool const backwards = beamDirection == 2 || beamDirection == 3;

            auto const swapAxis{[vertical](auto const &xVal, auto const &yVal){return vertical ? std::tie(xVal, yVal) : std::tie(yVal, xVal);}};

            auto const makeAttachment{[swapAxis](
                auto hasPlatform, auto platform, auto aOffset, auto bOffset)
            {
                auto [xOffset, yOffset] = swapAxis(aOffset, bOffset);
                return Attachment{
                    hasPlatform,
                    platform,
                    xOffset,
                    yOffset
                };
            }};
            auto const [aOffset, bOffset] = swapAxis(attachment.xOffset, attachment.yOffset);
            {
                auto const [aSize, bSize] = swapAxis(beamWidth, beamLength);
                beam = ButtonSegment{
                    makeAttachment(
                        attachment.hasPlatform,
                        attachment.platform,
                        aOffset - beamWidth/2,
                        bOffset - (backwards ? beamLength : 0)
                    ),
                    aSize,
                    bSize
                };
            }
            {
                auto const emitterLength = 400;
                auto const emitterWidth = 1200;
                auto const [aSize, bSize] = swapAxis(emitterWidth, emitterLength);
                emitter = ButtonSegment{
                    makeAttachment(
                        attachment.hasPlatform,
                        attachment.platform,
                        aOffset - emitterWidth/2,
                        bOffset - (backwards ? emitterLength : 0)
                    ),
                    aSize,
                    bSize
                };
            }
        }
        
        TimeDirection timeDirection;
        ButtonSegment beam;
        ButtonSegment emitter;
        int triggerID;
        int stateTriggerID;
        std::vector<int> extraTriggerIDs;

        int order_ranking() const noexcept final {
            return 5000;
        }
        
        bool operator==(ProtoButtonImpl const &o) const noexcept final {
            auto const &actual_other(*boost::polymorphic_downcast<ProtoStickyLaserSwitchImpl const*>(&o));
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
        virtual ~MutatorFrameStateImpl() noexcept = default;

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
        PickupFrameStateImpl(ProtoPickupImpl const &proto) :
            proto(&proto),
            active(true),
            justTaken(false)
        {}
        std::size_t clone_size() const noexcept final {
            return sizeof *this;
        }
        MutatorFrameStateImpl *perform_clone(void *memory) const final {
            return new (memory) PickupFrameStateImpl(*this);
        }
        ~PickupFrameStateImpl() final {}

        void addMutator(
            mp::std::vector<mp::std::vector<int>> const &triggerArrivals,
            mp::std::vector<Collision> const &collisions,
            mp::std::vector<MutatorArea> &mutators,
            mp::std::vector<MutatorFrameStateImpl *> &activeMutators
        ) final;

        void calculateGlitz(
            mt::std::vector<Glitz> &forwardsGlitz,
            mt::std::vector<Glitz> &reverseGlitz,
            mt::std::vector<GlitzPersister> &persistentGlitz) const final;
        void fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const final;
        boost::optional<Guy> effect(Guy const &guy) final;
        boost::optional<Box> effect(Box const &box) final {
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
        std::size_t clone_size() const final {
            return sizeof *this;
        }
        MutatorFrameStateImpl *perform_clone(void *memory) const final {
            return new (memory) SpikesFrameStateImpl(*this);
        }
        ~SpikesFrameStateImpl() final {}

        void addMutator(
            mp::std::vector<mp::std::vector<int>> const &triggerArrivals,
            mp::std::vector<Collision> const &collisions,
            mp::std::vector<MutatorArea> &mutators,
            mp::std::vector<MutatorFrameStateImpl *> &activeMutators
        ) final;

        void calculateGlitz(
            mt::std::vector<Glitz> &forwardsGlitz,
            mt::std::vector<Glitz> &reverseGlitz,
            mt::std::vector<GlitzPersister> &persistentGlitz) const final;
        void fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const final;
        boost::optional<Guy> effect(Guy const &guy) final;
        boost::optional<Box> effect(Box const &box) final {
            return box;
        }
    };


    struct ProtoPowerupImpl;

    struct PowerupFrameStateImpl final : MutatorFrameStateImpl {
    private:
        ProtoPowerupImpl const *proto;
        bool justTaken;
        bool active;
        PositionAndVelocity2D PnV;//If active, initialised by addMutator
    public:
        PowerupFrameStateImpl(ProtoPowerupImpl const &proto) noexcept :
            proto(&proto), justTaken(false), active(true)
        {}
        std::size_t clone_size() const final {
            return sizeof *this;
        }
        MutatorFrameStateImpl *perform_clone(void *memory) const final {
            return new (memory) PowerupFrameStateImpl(*this);
        }
        ~PowerupFrameStateImpl() final {}

        void addMutator(
            mp::std::vector<mp::std::vector<int>> const &triggerArrivals,
            mp::std::vector<Collision> const &collisions,
            mp::std::vector<MutatorArea> &mutators,
            mp::std::vector<MutatorFrameStateImpl *> &activeMutators
        ) final;

        void calculateGlitz(
            mt::std::vector<Glitz> &forwardsGlitz,
            mt::std::vector<Glitz> &reverseGlitz,
            mt::std::vector<GlitzPersister> &persistentGlitz) const final;
        void fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const final;
        boost::optional<Guy> effect(Guy const &guy) final;
        boost::optional<Box> effect(Box const &box) final {
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
        /*
        1000 ProtoPickupImpl
        2000 ProtoSpikesImpl
        3000 ProtoPowerupImpl
        */
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

        MutatorFrameState getFrameState(hg::memory_pool<hg::user_allocator_tbb_alloc> & pool) const final {
            return MutatorFrameState(new (pool) PickupFrameStateImpl(*this), pool);
        }
        std::size_t clone_size() const final {
            return sizeof *this;
        }
        ProtoMutatorImpl *perform_clone(void *memory) const final {
            return new (memory) ProtoPickupImpl(*this);
        }

        ~ProtoPickupImpl() final {}
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

        int order_ranking() const final {
            return 1000;
        }
        bool operator==(ProtoMutatorImpl const &o) const final {
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
        MutatorFrameState getFrameState(hg::memory_pool<hg::user_allocator_tbb_alloc> &pool) const final {
            return MutatorFrameState(new (pool) SpikesFrameStateImpl(*this, pool), pool);
        }
        std::size_t clone_size() const final {
            return sizeof *this;
        }
        ProtoMutatorImpl *perform_clone(void *memory) const final {
            return new (memory) ProtoSpikesImpl(*this);
        }

        ~ProtoSpikesImpl() final {}
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

        int order_ranking() const final {
            return 2000;
        }
        bool operator==(ProtoMutatorImpl const &o) const final {
            ProtoSpikesImpl const &actual_other(*boost::polymorphic_downcast<ProtoSpikesImpl const*>(&o));
            return comparison_tuple() == actual_other.comparison_tuple();
        }
    };

    struct ProtoPowerupImpl final : ProtoMutatorImpl {
    private:
        auto comparison_tuple() const noexcept {
            return std::tie(
                powerupType,
                timeDirection,
                attachment,
                width,
                height,
                triggerID
            );
        }
    public:
        MutatorFrameState getFrameState(hg::memory_pool<hg::user_allocator_tbb_alloc> &pool) const final {
            return MutatorFrameState(new (pool) PowerupFrameStateImpl(*this), pool);
        }
        std::size_t clone_size() const final {
            return sizeof *this;
        }
        ProtoMutatorImpl *perform_clone(void *memory) const final {
            return new (memory) ProtoPowerupImpl(*this);
        }

        ~ProtoPowerupImpl() final = default;
        explicit ProtoPowerupImpl(
            Powerup const powerupType,
            TimeDirection const timeDirection,
            Attachment const attachment,
            int const width,
            int const height,
            int const triggerID
        ) :
            powerupType(powerupType),
            timeDirection(timeDirection),
            attachment(attachment),
            width(width),
            height(height),
            triggerID(triggerID)
        {}
        Powerup powerupType;
        TimeDirection timeDirection;
        Attachment attachment;
        int width;
        int height;
        int triggerID;
        int order_ranking() const final {
            return 3000;
        }
        bool operator==(ProtoMutatorImpl const &o) const final {
            auto const &actual_other(*boost::polymorphic_downcast<ProtoPowerupImpl const*>(&o));
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

    struct ProtoGlitzImpl {
        virtual void calculateGlitz(
            mt::std::vector<Glitz> &forwardsGlitz,
            mt::std::vector<Glitz> &reverseGlitz,
            PhysicsAffectingStuff const &physicsAffectingStuff,
            mp::std::vector<mp::std::vector<int>> const &triggerArrivals,
            mp::std::map<std::size_t, mt::std::vector<int>> const& outputTriggers,
            Frame const *const currentFrame) const = 0;
        virtual std::size_t clone_size() const = 0;
        virtual ProtoGlitzImpl *perform_clone(void *memory) const = 0;
        ProtoGlitzImpl() = default;
        virtual ~ProtoGlitzImpl() = default;
        ProtoGlitzImpl(const ProtoGlitzImpl&) = default;
        ProtoGlitzImpl& operator=(const ProtoGlitzImpl&) = default;
        ProtoGlitzImpl(ProtoGlitzImpl&&) = default;
        ProtoGlitzImpl& operator=(ProtoGlitzImpl&&) = default;
        /*
        1000 ProtoWireGlitzImpl
        2000 ProtoBasicRectangleGlitzImpl
        3000 ProtoBasicTextGlitzImpl
        */
        virtual int order_ranking() const = 0;
        virtual bool operator==(ProtoGlitzImpl const &o) const = 0;
    };
    struct PlatformAndPos {
    private:
        auto comparison_tuple() const {
            return std::tie(platformId, pos);
        }
    public:
        std::optional<std::size_t> platformId;
        int pos;
        bool operator==(PlatformAndPos const &o) const {
            return comparison_tuple() == o.comparison_tuple();
        }
    };
    struct ProtoWireGlitzImpl final : ProtoGlitzImpl {
    private:
        auto comparison_tuple() const {
            return std::tie(
                triggerID_, 
                //hasTriggerClause_, triggerClause_,
                useTriggerArrival_, x1_, y1_, x2_, y2_
            );
        }
    public:
        ProtoWireGlitzImpl(
            std::optional<int> const triggerID,
            bool const hasTriggerClause,
            TriggerClause const triggerClause,
            bool const useTriggerArrival,
            PlatformAndPos const &x1,
            PlatformAndPos const &y1,
            PlatformAndPos const &x2,
            PlatformAndPos const &y2
        ) :
            triggerID_(triggerID),
            hasTriggerClause_(hasTriggerClause),
            triggerClause_(triggerClause),
            useTriggerArrival_(useTriggerArrival),
            x1_(x1),
            y1_(y1),
            x2_(x2),
            y2_(y2)
        {
        }
        void calculateGlitz(
            mt::std::vector<Glitz> &forwardsGlitz,
            mt::std::vector<Glitz> &reverseGlitz,
            PhysicsAffectingStuff const &physicsAffectingStuff,
            mp::std::vector<mp::std::vector<int>> const &triggerArrivals,
            mp::std::map<std::size_t, mt::std::vector<int>> const& outputTriggers,
            Frame const *const currentFrame) const final
        {
            auto const &collisions{ physicsAffectingStuff.collisions };
            auto const computePosition{[&collisions](
                PlatformAndPos const &p,
                auto const &getCollisionPos
                )
            {
                return p.pos + (
                    p.platformId.has_value()
                 && *p.platformId >= 0 //TODO: Pre-enforce platformId being in-bounds?
                 && *p.platformId < collisions.size() ?
                        getCollisionPos(collisions[*p.platformId]) :
                        0
                    );
            }};
            auto const getXpos{[](Collision const &c) {return c.getX();} };
            auto const getYpos{[](Collision const &c) {return c.getY();} };
            auto const x1{ computePosition(x1_, getXpos) };
            auto const y1{ computePosition(y1_, getYpos) };
            auto const x2{ computePosition(x2_, getXpos) };
            auto const y2{ computePosition(y2_, getYpos) };

            DynamicArea const obj{
                x1 < x2 ? x1 : x2,
                y1 < y2 ? y1 : y2,
                0,
                0,
                x1 < x2 ? x2 - x1 : x1 - x2,
                y1 < y2 ? y2 - y1 : y1 - y2,
                TimeDirection::REVERSE
            };

            auto const active
            {
                [&]{
                    if (triggerID_.has_value()) {
                        if (useTriggerArrival_) {
                            //TODO: Pre-enforce triggerID being in-bounds and
                            //defaults having at least one value?
                            if (*triggerID_ < 0 || *triggerID_ >= triggerArrivals.size()){
                                return false;
                            }
                            if (triggerArrivals[*triggerID_].size() < 1) {
                                return false;
                            }
                            return triggerArrivals[*triggerID_][0] > 0;
                        }
                        else {
                            auto const outputTriggerIt{ outputTriggers.find(*triggerID_) };
                            if (outputTriggerIt == outputTriggers.end()) {
                                return false;
                            }
                            if (outputTriggerIt->second.size() < 1) {
                                return false;
                            }
                            return outputTriggerIt->second[0] > 0;
                        }
                        
                    }
                    else if (hasTriggerClause_) {
                        if (useTriggerArrival_) {
                            return triggerClause_.GetOutput(triggerArrivals, getFrameNumber(currentFrame)) > 0;
                        }
                        else {
                            return triggerClause_.GetOutput(outputTriggers, getFrameNumber(currentFrame)) > 0;
                        }
                    }
                    else {
                        return false;
                    }
                }()
            };
            auto const colour{active ? asPackedColour(0, 180, 0) : asPackedColour(180, 0, 0)};
            auto [forGlitz, revGlitz] = calculateBidirectionalGlitz(1500, obj, colour, colour);

            forwardsGlitz.emplace_back(std::move(forGlitz));
            reverseGlitz.emplace_back(std::move(revGlitz));
        }
        std::size_t clone_size() const final {
            return sizeof *this;
        }
        ProtoWireGlitzImpl *perform_clone(void *memory) const final {
            return new (memory) ProtoWireGlitzImpl(*this);
        }
        //~ProtoGlitzImpl() final = default;
        int order_ranking() const final {
            return 1000;
        }
        bool operator==(ProtoGlitzImpl const &o) const final {
            ProtoWireGlitzImpl const &actual_other(*boost::polymorphic_downcast<ProtoWireGlitzImpl const*>(&o));
            return comparison_tuple() == actual_other.comparison_tuple();
        }
        private:
        std::optional<int> triggerID_;
        bool hasTriggerClause_;
        TriggerClause triggerClause_;
        bool useTriggerArrival_;
        PlatformAndPos x1_;
        PlatformAndPos y1_;
        PlatformAndPos x2_;
        PlatformAndPos y2_;
    };

    struct ProtoBasicRectangleGlitzImpl final : ProtoGlitzImpl {
    private:
        auto comparison_tuple() const {
            return std::tie(
                colour,
                layer,
                x,
                y,
                width,
                height
            );
        }
    public:
        ProtoBasicRectangleGlitzImpl(
            unsigned const colour,
            int const layer,
            int const x,
            int const y,
            int const width,
            int const height
        ) :
            colour(colour),
            layer(layer),
            x(x),
            y(y),
            width(width),
            height(height)
        {
        }
        void calculateGlitz(
            mt::std::vector<Glitz> &forwardsGlitz,
            mt::std::vector<Glitz> &reverseGlitz,
            PhysicsAffectingStuff const &physicsAffectingStuff,
            mp::std::vector<mp::std::vector<int>> const &triggerArrivals,
            mp::std::map<std::size_t, mt::std::vector<int>> const& outputTriggers,
            Frame const *const currentFrame) const final
        {
            DynamicArea const obj{
                x,
                y,
                0,
                0,
                width,
                height,
                TimeDirection::REVERSE
            };

            auto[forGlitz, revGlitz] = calculateBidirectionalGlitz(layer, obj, colour, colour);

            forwardsGlitz.emplace_back(std::move(forGlitz));
            reverseGlitz.emplace_back(std::move(revGlitz));
        }
        std::size_t clone_size() const final {
            return sizeof *this;
        }
        ProtoBasicRectangleGlitzImpl *perform_clone(void *memory) const final {
            return new (memory) ProtoBasicRectangleGlitzImpl(*this);
        }
        int order_ranking() const final {
            return 2000;
        }
        bool operator==(ProtoGlitzImpl const &o) const final {
            auto const &actual_other(*boost::polymorphic_downcast<ProtoBasicRectangleGlitzImpl const*>(&o));
            return comparison_tuple() == actual_other.comparison_tuple();
        }
    private:
        unsigned colour;
        int layer;
        int x;
        int y;
        int width;
        int height;
    };

    struct ProtoBasicTextGlitzImpl final : ProtoGlitzImpl {
    private:
        auto comparison_tuple() const {
            return std::tie(
                glitz
            );
        }
    public:
        ProtoBasicTextGlitzImpl(
            TextGlitz glitz
        ) :
            glitz(std::move(glitz))
        {
        }
        void calculateGlitz(
            mt::std::vector<Glitz> &forwardsGlitz,
            mt::std::vector<Glitz> &reverseGlitz,
            PhysicsAffectingStuff const &physicsAffectingStuff,
            mp::std::vector<mp::std::vector<int>> const &triggerArrivals,
            mp::std::map<std::size_t, mt::std::vector<int>> const& outputTriggers,
            Frame const *const currentFrame) const final
        {
            forwardsGlitz.emplace_back(mt::std::make_unique<TextGlitz>(glitz));
            reverseGlitz.emplace_back(mt::std::make_unique<TextGlitz>(glitz));
        }
        std::size_t clone_size() const final {
            return sizeof *this;
        }
        ProtoBasicTextGlitzImpl *perform_clone(void *memory) const final {
            return new (memory) ProtoBasicTextGlitzImpl(*this);
        }
        int order_ranking() const final {
            return 3000;
        }
        bool operator==(ProtoGlitzImpl const &o) const final {
            auto const &actual_other(*boost::polymorphic_downcast<ProtoBasicTextGlitzImpl const*>(&o));
            return comparison_tuple() == actual_other.comparison_tuple();
        }
    private:
        TextGlitz glitz;
    };


    struct ProtoGlitz final {
    private:
        clone_ptr<ProtoGlitzImpl, memory_source_clone<ProtoGlitzImpl, multi_thread_memory_source>> pimpl_;
        
        typedef
            std::tuple<
            decltype(pimpl_->order_ranking()),
            ProtoGlitzImpl const &>
            comparison_tuple_type;
        comparison_tuple_type comparison_tuple() const {
            return comparison_tuple_type(pimpl_->order_ranking(), *pimpl_);
        }
        
    public:
        explicit ProtoGlitz(mt::std::unique_ptr<ProtoGlitzImpl> impl)
            : pimpl_(impl.release())
        {
            assert(pimpl_);
        }
        void calculateGlitz(
            mt::std::vector<Glitz> &forwardsGlitz,
            mt::std::vector<Glitz> &reverseGlitz,
            PhysicsAffectingStuff const &physicsAffectingStuff,
            mp::std::vector<mp::std::vector<int>> const &triggerArrivals,
            mp::std::map<std::size_t, mt::std::vector<int>> const& outputTriggers,
            Frame const *const currentFrame) const
        {
            return pimpl_->calculateGlitz(forwardsGlitz, reverseGlitz, physicsAffectingStuff, triggerArrivals,outputTriggers, currentFrame);
        }
        bool operator==(ProtoGlitz const &o) const {
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
            std::vector<ProtoTriggerMod> const &protoTriggerMods,
            std::vector<ProtoGlitz> const &protoGlitzs,
            memory_pool<user_allocator_tbb_alloc> &pool,
            OperationInterrupter &interrupter);

        PhysicsAffectingStuff //TODO: Return a reference to the stored PhysicsAffectingStuff, rather than copying it?
            calculatePhysicsAffectingStuff(
                Frame const *currentFrame,
                boost::transformed_range<
                    GetBase<TriggerDataConstPtr>,
                    mt::boost::container::vector<TriggerDataConstPtr> const> const &triggerArrivals) final;

        bool shouldArrive(Guy const &potentialArriver) final;
        bool shouldArrive(Box const &potentialArriver) final;

        bool shouldPort(
            int responsiblePortalIndex,
            Guy const &potentialPorter,
            bool porterActionedPortal) final;
        bool shouldPort(
            int responsiblePortalIndex,
            Box const &potentialPorter,
            bool porterActionedPortal) final;

        boost::optional<Guy> mutateObject(
            mp::std::vector<int> const &responsibleMutatorIndices,
            Guy const &objectToManipulate) final;
        boost::optional<Box> mutateObject(
            mp::std::vector<int> const &responsibleMutatorIndices,
            Box const &objectToManipulate) final;

        DepartureInformation getDepartureInformation(
            mt::std::map<Frame*, ObjectList<Normal>> const &departures,
            Frame *currentFrame) final;
        ~SimpleConfiguredTriggerFrameState() noexcept final;
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

        PhysicsAffectingStuff physicsAffectingStuff_;

        std::vector<ProtoCollision> const &protoCollisions_;
        std::vector<ProtoPortal> const &protoPortals_;
        std::vector<ProtoTriggerMod> const &protoTriggerMods_;
        std::vector<ProtoGlitz> const &protoGlitzs_;

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
                protoTriggerMods_,
                protoGlitzs_,
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
        TriggerFrameState getFrameState(memory_pool<user_allocator_tbb_alloc> &pool, OperationInterrupter &interrupter) const final{
            return TriggerFrameState(
                new (pool) SimpleConfiguredTriggerFrameState(
                    triggerOffsetsAndDefaults_,
                    arrivalLocationsSize_,
                    protoCollisions_,
                    protoPortals_,
                    protoButtons_,
                    protoMutators_,
                    protoTriggerMods_,
                    protoGlitzs_,
                    pool,
                    interrupter));
        }
        TriggerSystemImplementation *clone() const final
        {
            return new SimpleConfiguredTriggerSystem(*this);
        }
        bool operator==(TriggerSystemImplementation const &o) const final
        {
            SimpleConfiguredTriggerSystem const &actual_other(*boost::polymorphic_downcast<SimpleConfiguredTriggerSystem const*>(&o));
            return comparison_tuple() == actual_other.comparison_tuple();
        }
        int order_ranking() const final
        {
            return 2000;
        }
    private:

        std::vector<ProtoPortal> protoPortals_;
        std::vector<ProtoCollision> protoCollisions_;
        //Rename Collision -> Collider?
        //TODO: Use boost polymorphic collection rather than vector of clone_ptr?
        std::vector<ProtoButton> protoButtons_;
        std::vector<ProtoMutator> protoMutators_;
        std::vector<ProtoTriggerMod> protoTriggerMods_;
        std::vector<ProtoGlitz> protoGlitzs_;

        std::vector<
            std::pair<int, std::vector<int>>
        > triggerOffsetsAndDefaults_;
        std::size_t arrivalLocationsSize_;
    };

}
#endif //HG_SIMPLE_CONFIGURED_TRIGGER_SYSTEM_H
