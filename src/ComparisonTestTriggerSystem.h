#ifndef HG_COMPARISON_TEST_TRIGGER_SYSTEM_H
#define HG_COMPARISON_TEST_TRIGGER_SYSTEM_H
#include "TriggerSystemImplementation.h"
#include "DirectLuaTriggerSystem.h"
#include "SimpleConfiguredTriggerSystem.h"
#include "TriggerSystem.h"
#include <gsl/gsl>

namespace hg {
class OperationInterrupter;
class ComparisonTestTriggerFrameState final :
    public TriggerFrameStateImplementation
{
    public:
    ComparisonTestTriggerFrameState(
        TriggerFrameState &&directLuaFrameState,
        TriggerFrameState &&simpleConfiguredFrameState
        )
        :directLuaFrameState(std::move(directLuaFrameState)),
        simpleConfiguredFrameState(std::move(simpleConfiguredFrameState))
    {
    }
    
    PhysicsAffectingStuff
        calculatePhysicsAffectingStuff(
            Frame const *currentFrame,
            boost::transformed_range<
                GetBase<TriggerDataConstPtr>,
                mt::boost::container::vector<TriggerDataConstPtr> const> const &triggerArrivals) override
    {
        auto physicsAffectingStuffA = directLuaFrameState.calculatePhysicsAffectingStuff(currentFrame, triggerArrivals);
        auto physicsAffectingStuffB = simpleConfiguredFrameState.calculatePhysicsAffectingStuff(currentFrame, triggerArrivals);

        if (physicsAffectingStuffA.additionalBoxes != physicsAffectingStuffB.additionalBoxes) {
            Expects(false && "physicsAffectingStuffA.additionalBoxes != physicsAffectingStuffB.additionalBoxes");
            throw std::exception("calculatePhysicsAffectingStuff.additionalBoxes mismatch");
        }
        if (physicsAffectingStuffA.portals != physicsAffectingStuffB.portals) {
            Expects(false && "physicsAffectingStuffA.portals != physicsAffectingStuffB.portals");
            throw std::exception("calculatePhysicsAffectingStuff.portals mismatch");
        }
        if (physicsAffectingStuffA.collisions != physicsAffectingStuffB.collisions) {
            Expects(false && "physicsAffectingStuffA.collisions != physicsAffectingStuffB.collisions");
            throw std::exception("calculatePhysicsAffectingStuff.collisions mismatch");
        }
        if (physicsAffectingStuffA.mutators != physicsAffectingStuffB.mutators) {
            Expects(false && "physicsAffectingStuffA.mutators != physicsAffectingStuffB.mutators");
            throw std::exception("calculatePhysicsAffectingStuff.mutators mismatch");
        }
        if (physicsAffectingStuffA.arrivalLocations != physicsAffectingStuffB.arrivalLocations) {
            Expects(false && "physicsAffectingStuffA.arrivalLocations != physicsAffectingStuffB.arrivalLocations");
            throw std::exception("calculatePhysicsAffectingStuff.arrivalLocations mismatch");
        }

        return physicsAffectingStuffA;
    }

    bool shouldArrive(Guy const &potentialArriver) override {
        auto shouldArriveA = directLuaFrameState.shouldArrive(potentialArriver);
        auto shouldArriveB = simpleConfiguredFrameState.shouldArrive(potentialArriver);

        if (shouldArriveA != shouldArriveB) {
            assert(false && "shouldArriveA != shouldArriveB Guy");
            throw std::exception("shouldArrive<Guy> mismatch");
        }

        return shouldArriveA;
    }
    bool shouldArrive(Box const &potentialArriver) override {
        auto shouldArriveA = directLuaFrameState.shouldArrive(potentialArriver);
        auto shouldArriveB = simpleConfiguredFrameState.shouldArrive(potentialArriver);

        if (shouldArriveA != shouldArriveB) {
            assert(false && "shouldArriveA != shouldArriveB Box");
            throw std::exception("shouldArrive<Box> mismatch");
        }

        return shouldArriveA;
    }

    bool shouldPort(
        int responsiblePortalIndex,
        Guy const &potentialPorter,
        bool porterActionedPortal) override
    {
        auto shouldPortA = directLuaFrameState.shouldPort(responsiblePortalIndex, potentialPorter, porterActionedPortal);
        auto shouldPortB = simpleConfiguredFrameState.shouldPort(responsiblePortalIndex, potentialPorter, porterActionedPortal);

        if (shouldPortA != shouldPortB) {
            assert(false && "shouldPortA != shouldPortB Guy");
            throw std::exception("shouldPort<Guy> mismatch");
        }
        return shouldPortA;
    }
    bool shouldPort(
        int responsiblePortalIndex,
        Box const &potentialPorter,
        bool porterActionedPortal) override
    {
        auto shouldPortA = directLuaFrameState.shouldPort(responsiblePortalIndex, potentialPorter, porterActionedPortal);
        auto shouldPortB = simpleConfiguredFrameState.shouldPort(responsiblePortalIndex, potentialPorter, porterActionedPortal);

        if (shouldPortA != shouldPortB) {
            assert(false && "shouldPortA != shouldPortB Box");
            throw std::exception("shouldPort<Box> mismatch");
        }
        return shouldPortA;
    }
    
    boost::optional<Guy> mutateObject(
        mp::std::vector<int> const &responsibleMutatorIndices,
        Guy const &objectToManipulate) override {
        auto mutatedA = directLuaFrameState.mutateObject(responsibleMutatorIndices, objectToManipulate);
        auto mutatedB = simpleConfiguredFrameState.mutateObject(responsibleMutatorIndices, objectToManipulate);

        if (mutatedA != mutatedB) {
            assert(false && "mutatedA != mutatedB Guy");
            throw std::exception("mutateObject<Guy> mismatch");
        }
        return mutatedA;
    }
    boost::optional<Box> mutateObject(
        mp::std::vector<int> const &responsibleMutatorIndices,
        Box const &objectToManipulate) override {
        auto mutatedA = directLuaFrameState.mutateObject(responsibleMutatorIndices, objectToManipulate);
        auto mutatedB = simpleConfiguredFrameState.mutateObject(responsibleMutatorIndices, objectToManipulate);

        if (mutatedA != mutatedB) {
            assert(false && "mutatedA != mutatedB Box");
            throw std::exception("mutateObject<Box> mismatch");
        }
        return mutatedA;
    }

    DepartureInformation getDepartureInformation(
        mt::std::map<Frame*, ObjectList<Normal>> const &departures,
        Frame *currentFrame) override
    {
        auto departureInfoA = directLuaFrameState.getDepartureInformation(departures, currentFrame);
        auto departureInfoB = simpleConfiguredFrameState.getDepartureInformation(departures, currentFrame);

        if (departureInfoA.triggerDepartures != departureInfoB.triggerDepartures) {
            assert(false && "departureInfoA.triggerDepartures != departureInfoB.triggerDepartures");
            throw std::exception("getDepartureInformation.triggerDepartures mismatch");
        }
        if (departureInfoA.forwardsGlitz != departureInfoB.forwardsGlitz) {
            assert(false && "departureInfoA.forwardsGlitz != departureInfoB.forwardsGlitz");
            throw std::exception("getDepartureInformation.forwardsGlitz mismatch");
        }
        if (departureInfoA.reverseGlitz != departureInfoB.reverseGlitz) {
            assert(false && "departureInfoA.reverseGlitz != departureInfoB.reverseGlitz");
            throw std::exception("getDepartureInformation.reverseGlitz mismatch");
        }
        if (departureInfoA.additionalGlitzPersisters != departureInfoB.additionalGlitzPersisters) {
            assert(false && "departureInfoA.additionalGlitzPersisters != departureInfoB.additionalGlitzPersisters");
            throw std::exception("getDepartureInformation.additionalGlitzPersisters mismatch");
        }
        if (departureInfoA.additionalBoxDepartures != departureInfoB.additionalBoxDepartures) {
            assert(false && "departureInfoA.additionalBoxDepartures != departureInfoB.additionalBoxDepartures");
            throw std::exception("getDepartureInformation.additionalBoxDepartures mismatch");
        }

        return departureInfoA;
    }
    ~ComparisonTestTriggerFrameState() noexcept override {}
private:
    
    TriggerFrameState directLuaFrameState;
    TriggerFrameState simpleConfiguredFrameState;


    ComparisonTestTriggerFrameState(ComparisonTestTriggerFrameState &o) = delete;
    ComparisonTestTriggerFrameState &operator=(ComparisonTestTriggerFrameState &o) = delete;
    ComparisonTestTriggerFrameState(ComparisonTestTriggerFrameState &&o) = delete;
    ComparisonTestTriggerFrameState &operator=(ComparisonTestTriggerFrameState &&o) = delete;
};
class ComparisonTestTriggerSystem final :
    public TriggerSystemImplementation
{

    auto comparison_tuple() const -> decltype(auto)
    {
        //Doesn't include luaStates_, since luaStates_ is just a cache.
        return std::tie(systemA, systemB);
    }
public:
    ComparisonTestTriggerSystem(
        TriggerSystem &&systemA,
        TriggerSystem &&systemB
    ) :
        systemA(std::move(systemA)),
        systemB(std::move(systemB))
    {}
    TriggerFrameState getFrameState(memory_pool<user_allocator_tbb_alloc> &pool, OperationInterrupter &interrupter) const {
        return TriggerFrameState(
            new (pool) ComparisonTestTriggerFrameState(
                systemA.getFrameState(pool, interrupter),
                systemB.getFrameState(pool, interrupter)));
    }
    TriggerSystemImplementation *clone() const override
    {
        return new ComparisonTestTriggerSystem(*this);
    }
    bool operator==(TriggerSystemImplementation const &o) const override
    {
        ComparisonTestTriggerSystem const &actual_other(*boost::polymorphic_downcast<ComparisonTestTriggerSystem const*>(&o));
        return comparison_tuple() == actual_other.comparison_tuple();
    }
    int order_ranking() const override
    {
        return 10000;
    }
private:
    TriggerSystem systemA;
    TriggerSystem systemB;
};
}
#endif //HG_COMPARISON_TEST_TRIGGER_SYSTEM_H
