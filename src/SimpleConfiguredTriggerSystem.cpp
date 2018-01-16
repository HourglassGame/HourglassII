#include "SimpleConfiguredTriggerSystem.h"
#include "Maths.h"
namespace hg {
    struct PositionAndVelocity {
        int position;
        int velocity;
    };
    PositionAndVelocity solvePDEquation(AxisCollisionDestination const &destination, int const position, double velocity)
    {
        auto const desiredPosition = destination.desiredPosition;
        auto const acceleration = destination.acceleration;
        auto const deceleration = destination.deceleration;
        if (desiredPosition != position) {
            if (abs(desiredPosition-position) <= abs(velocity)
             && abs(velocity) <= deceleration)
            {
                velocity = desiredPosition - position;
            }
            else
            {
                auto const direction = sign(position-desiredPosition);
                if (velocity * direction >= 0) {
                    velocity -= direction*deceleration;
                    if (velocity * direction < 0) {
                        velocity = 0;
                    }
                }
                else if (abs(position - desiredPosition)
                    >
                    (pow(velocity - direction * acceleration, 2) * 3 / (2 * deceleration)))
                {
                    velocity -= direction * acceleration;
                }
                else if (abs(position - desiredPosition) > pow(velocity, 2)*3/(2*deceleration))
                {
                    velocity = -direction * sqrt(abs(position-desiredPosition)*deceleration*2/3);
                
                }
                else {
                    velocity += direction*deceleration;
                }
            }
        }
        else {
            if (abs(velocity) <= deceleration) {
                velocity = 0;
            }
            else {
                velocity += sign(velocity)*deceleration;
            }
        }
        auto const maxSpeed = destination.maxSpeed;
        if (abs(velocity) > maxSpeed) {
            velocity = sign(velocity) * maxSpeed;
        }

        //TODO: float/int maths consistency between C++ and Lua
        velocity = velocity >= 0 ? floor(velocity) : ceil(velocity);

        return {position + static_cast<int>(velocity), static_cast<int>(velocity)};
    }

    void calculateCollisions(
        mp::std::vector<Collision> &collisions,
        std::vector<ProtoCollision> const &protoCollisions,
        mp::std::vector<mp::std::vector<int>> const &triggerArrivals,
        Frame const *const currentFrame
    )
    {
        auto const calculateCollision = [&triggerArrivals](ProtoCollision const &protoCollision){
            /*
            //TODO
            if (protoCollision.rawCollisionFunction) {
                return protoCollision.rawCollisionFunction(triggerArrivals, outputTriggers, frameNumber);
            }
            */


            bool const active =
                (protoCollision.hasButtonTriggerID && triggerArrivals[protoCollision.buttonTriggerID][0] > 0)/*
              ||(protoCollisions.hasTriggerFunction() && self.getTriggerFunction(triggerArrivals, frameNumber)*/ //TODO
                ;

            CollisionDestination const &destination =
                active ? protoCollision.onDestination : protoCollision.offDestination;

            mp::std::vector<int> const &lastStateTrigger = triggerArrivals[protoCollision.lastStateTriggerID];
            auto const horizontalPosAndVel = solvePDEquation(
                destination.xDestination, lastStateTrigger[0], lastStateTrigger[2]);

            auto const verticalPosAndVel = solvePDEquation(
                destination.yDestination, lastStateTrigger[1], lastStateTrigger[3]);

            return Collision(
                /*int x*/horizontalPosAndVel.position,/*int y*/verticalPosAndVel.position,
                /*int xspeed*/horizontalPosAndVel.velocity, /*int yspeed*/verticalPosAndVel.velocity,
                /*int prevXspeed*/lastStateTrigger[2], /*int prevYspeed*/lastStateTrigger[3],
                /*int width*/protoCollision.width, /*int height*/protoCollision.height,
                /*TimeDirection timeDirection*/protoCollision.timeDirection
            );
        };

        boost::push_back(collisions, protoCollisions | boost::adaptors::transformed(calculateCollision));
    }

    ArrivalLocation calculateArrivalLocation(PortalArea const &portal)
    {
        return
        {
            portal.getX(),
            portal.getY(),
            portal.getXspeed(),
            portal.getYspeed(),
            portal.getTimeDirection()
        };
    }

    std::tuple<int, int, int, int> snapAttachment(TimeDirection objectTimeDirection, Attachment const &attachment, mp::std::vector<Collision> const &collisions)
    {
        if (attachment.hasPlatform) {
            //TODO: Properly check/report this assert
            assert(attachment.platform >= 0 && attachment.platform < collisions.size());
            auto const &collision = collisions[attachment.platform];
            if (collision.getTimeDirection() == objectTimeDirection) {
                return {
                    collision.getX() + attachment.xOffset,
                    collision.getY() + attachment.yOffset,
                    collision.getXspeed(),
                    collision.getYspeed(),
                };
            }
            else {
                return {
                    collision.getX()-collision.getXspeed() + attachment.xOffset,
                    collision.getY()-collision.getYspeed() + attachment.yOffset,
                    -collision.getXspeed(),
                    -collision.getYspeed(),
                };
            }
        }
        else {
            return {
                attachment.xOffset,
                attachment.yOffset,
                0,
                0,
            };
        }
    }

    std::tuple<PortalArea, bool> calculatePortal(ProtoPortal const &protoPortal, mp::std::vector<Collision> const &collisions)
    {
        auto [x, y, xspeed, yspeed] = snapAttachment(protoPortal.timeDirection, protoPortal.attachment, collisions);
        PortalArea retPortal{
            protoPortal.index,//int index,
            x,// int x,
            y,// int y,
            protoPortal.xaim,// int xaim,
            protoPortal.yaim,// int yaim,
            protoPortal.width, //int width,
            protoPortal.height, //int height,
            xspeed,// int xspeed,
            yspeed,// int yspeed,
            protoPortal.collisionOverlap,// int collisionOverlap,
            protoPortal.timeDirection,// TimeDirection timeDirection,
            protoPortal.destinationIndex,// int destinationIndex,
            protoPortal.xDestination,// int xDestination,
            protoPortal.yDestination,// int yDestination,
            protoPortal.relativeTime,// bool relativeTime,
            protoPortal.timeDestination,// int timeDestination,
            protoPortal.relativeDirection,// bool relativeDirection,
            protoPortal.destinationDirection,// TimeDirection destinationDirection,
            protoPortal.illegalDestination,// int illegalDestination,
            protoPortal.fallable,// bool fallable,
            protoPortal.isLaser,// bool isLaser,
            protoPortal.winner// bool winner
        };
        if (protoPortal.winner) {
            //Unnecessary...?
        }
        //TODO: if protoPortal.triggerFunction, call triggerFunction!

        return {
            retPortal, true
        };
    }

    void calculatePortals(
        mp::std::vector<PortalArea> &portals,
        mp::std::vector<ArrivalLocation> &arrivalLocations,
        mt::std::vector<Glitz> &forwardsGlitz,
        mt::std::vector<Glitz> &reverseGlitz,
        std::vector<ProtoPortal> const &protoPortals,
        mp::std::vector<Collision> const &collisions,
        mp::std::vector<mp::std::vector<int>> const &apparentTriggers,
        Frame const *const currentFrame
    )
    {
        for (auto &&protoPortal : protoPortals) {
            auto [portal, active] = calculatePortal(protoPortal, collisions);
            if (!protoPortal.isLaser) {
                //calculatePortalGlitz(portal, forwardsGlitz, reverseGlitz, active);//TODO
            }
            arrivalLocations.push_back(calculateArrivalLocation(portal));
            if (active) {
                portals.push_back(portal);
            }
        }
    }
    void calculateButtonPositionsAndVelocities(
        mp::std::vector<ButtonFrameState> &buttonFrameStates,
        mp::std::vector<Collision> const &collisions
        )
    {
        for (auto &&buttonFrameState : buttonFrameStates) {
            buttonFrameState.calcPnV(collisions);
        }
    }
    void fillCollisionTriggers(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers, std::vector<ProtoCollision> const &protoCollisions, mp::std::vector<Collision> const &collisions) {
        for (std::size_t i{0}, sz{protoCollisions.size()}; i != sz; ++i) {
            assert(protoCollisions.size() < protoCollisions[i].lastStateTriggerID);
            assert(collisions.size() == sz);
            auto const &coll = collisions[i];
            outputTriggers[protoCollisions[i].lastStateTriggerID] = mt::std::vector<int>{coll.getX(), coll.getY(), coll.getXspeed(), coll.getYspeed()};
        }
    }

    void calculateButtonStates(
        mp::std::vector<ButtonFrameState> &buttonFrameStates,
        mt::std::map<Frame *, ObjectList<Normal>> const &departures,
        mp::std::vector<mp::std::vector<int>> const &triggerArrivals)
    {
        for (auto &&buttonFrameState : buttonFrameStates) {
            buttonFrameState.updateState(departures, triggerArrivals);
        }
    }

    void calculateMutators(
        mp::std::vector<MutatorArea> &mutators,
        mp::std::vector<MutatorFrameState *> &activeMutators,
        mp::std::vector<MutatorFrameState> &mutatorFrameStates,
        mp::std::vector<Collision> const &collisions,
        mp::std::vector<mp::std::vector<int>> const &triggerArrivals)
    {
        assert(mutators.empty());
        assert(activeMutators.empty()); 
        for (auto &mutatorFrameState : mutatorFrameStates) {
            mutatorFrameState.addMutator(triggerArrivals, collisions, mutators, activeMutators);
        }
    }

    SimpleConfiguredTriggerFrameState::SimpleConfiguredTriggerFrameState(
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
        OperationInterrupter &interrupter)
      :
        pool_(pool),
        interrupter_(interrupter),
        triggerOffsetsAndDefaults_(triggerOffsetsAndDefaults),
        arrivalLocationsSize_(arrivalLocationsSize),
        outputTriggers_(pool),
        forwardsGlitz_(),
        reverseGlitz_(),
        buttonFrameStates_(pool),
        mutatorFrameStates_(pool),
        activeMutators_(pool),
        triggerArrivals_(pool),
        protoCollisions_(protoCollisions),
        protoPortals_(protoPortals)
    {
        boost::push_back(buttonFrameStates_, protoButtons | boost::adaptors::transformed([](auto const &a){return a.getFrameState();}));
        boost::push_back(mutatorFrameStates_, protoMutators | boost::adaptors::transformed([](auto const &a){return a.getFrameState();}));
    }

    PhysicsAffectingStuff
        SimpleConfiguredTriggerFrameState::calculatePhysicsAffectingStuff(
            Frame const *const currentFrame,
            boost::transformed_range<
                GetBase<TriggerDataConstPtr>,
                mt::boost::container::vector<TriggerDataConstPtr> const> const &triggerArrivals)
    {
#if 0
        local retv = {} //Done

        tempStore.outputTriggers = {}
        tempStore.forwardsGlitz = {}
        tempStore.reverseGlitz = {}
        tempStore.persistentGlitz = {}
        tempStore.frameNumber = frameNumber
        tempStore.triggerArrivals = triggerArrivals

        retv.additionalBoxes = {}
        tempStore.additionalEndBoxes = {}

        if tempStore.triggerManipulationFunction then
            tempStore.triggerManipulationFunction(triggerArrivals, tempStore.outputTriggers, tempStore.frameNumber)
        end

        retv.collisions = calculateCollisions(tempStore.protoCollisions, triggerArrivals, tempStore.outputTriggers, tempStore.frameNumber)//Mostly done

        retv.mutators, tempStore.activeMutators = calculateMutators(tempStore.protoMutators, retv.collisions, triggerArrivals)//mostly done

        local portals, arrivalLocations = calculatePortals(
            tempStore.forwardsGlitz,
            tempStore.reverseGlitz,
            tempStore.protoPortals,
            tempStore.protoLasers,
            retv.collisions,
            triggerArrivals,
            tempStore.frameNumber
        ) //done?

        retv.portals = portals //done?
        retv.arrivalLocations = arrivalLocations//done?

        calculateButtonPositionsAndVelocities(tempStore.protoButtons, retv.collisions) //partial done

        fillCollisionTriggers(tempStore.outputTriggers, tempStore.protoCollisions, retv.collisions)//done?

        for collision in list_iter(retv.collisions) do
            local forwardsGlitz, reverseGlitz = calculateCollisionGlitz(collision)
            table.insert(tempStore.forwardsGlitz, forwardsGlitz)
            table.insert(tempStore.reverseGlitz, reverseGlitz)
        end

        if tempStore.protoBoxCreators then
            for protoBoxCreator in list_iter(tempStore.protoBoxCreators) do
                protoBoxCreator:calcPnV(retv.collisions)
                protoBoxCreator : calculateGlitz(tempStore.forwardsGlitz, tempStore.reverseGlitz)
                protoBoxCreator : getAdditionalBoxes(tempStore.triggerArrivals, tempStore.frameNumber, retv.additionalBoxes)
            end
        end

        tempStore.physicsAffectingStuff = retv

        return retv
#endif
        triggerArrivals_ = calculateApparentTriggers(triggerOffsetsAndDefaults_, triggerArrivals, pool_);

        PhysicsAffectingStuff retv(pool_);
        /*
        mp::std::vector<Box> additionalBoxes;
        mp::std::vector<PortalArea> portals;
        mp::std::vector<Collision> collisions;
        mp::std::vector<MutatorArea> mutators;
        //guaranteed to always contain elements at each index that could possibly be indexed
        //ie- arrivalLocations will always be the same length for a particular TriggerSystem
        mp::std::vector<ArrivalLocation> arrivalLocations;
        */
        calculateCollisions(retv.collisions, protoCollisions_, triggerArrivals_, /*outputTriggers,*/ currentFrame);

        calculateMutators(retv.mutators, activeMutators_, mutatorFrameStates_, retv.collisions, triggerArrivals_);

        calculatePortals(
            retv.portals,
            retv.arrivalLocations,
            forwardsGlitz_,
            reverseGlitz_,
            protoPortals_,
            retv.collisions,
            triggerArrivals_,
            currentFrame
        );

        calculateButtonPositionsAndVelocities(buttonFrameStates_, retv.collisions);

        fillCollisionTriggers(outputTriggers_, protoCollisions_, retv.collisions);

        return retv;
    }



    TriggerFrameStateImplementation::DepartureInformation SimpleConfiguredTriggerFrameState::getDepartureInformation(
        mt::std::map<Frame*, ObjectList<Normal>> const &departures,
        Frame *currentFrame)
    {
#if 0
        calculateButtonStates(tempStore.protoButtons, departures, tempStore.triggerArrivals)

        for protoButton in list_iter(tempStore.protoButtons) do
            protoButton:calculateGlitz(tempStore.forwardsGlitz, tempStore.reverseGlitz, tempStore.persistentGlitz)
            protoButton:fillTrigger(tempStore.outputTriggers)
        end

        for protoMutator in list_iter(tempStore.protoMutators) do
            protoMutator:calculateGlitz(tempStore.forwardsGlitz, tempStore.reverseGlitz, tempStore.persistentGlitz)
            protoMutator:fillTrigger(tempStore.outputTriggers)
        end

        if tempStore.protoGlitz then
            for protoGlitz in list_iter(tempStore.protoGlitz) do
                protoGlitz:calculateGlitz(tempStore.forwardsGlitz, tempStore.reverseGlitz, tempStore.physicsAffectingStuff, tempStore.triggerArrivals, tempStore.outputTriggers)
            end
        end

        return
            tempStore.outputTriggers,
            tempStore.forwardsGlitz,
            tempStore.reverseGlitz,
            tempStore.persistentGlitz,
            tempStore.additionalEndBoxes
#endif
        mt::std::vector<Glitz> forwardsGlitz;
        mt::std::vector<Glitz> reverseGlitz;
        mt::std::vector<GlitzPersister> additionalGlitzPersisters;
        mt::std::vector<ObjectAndTime<Box, Frame *>> additionalBoxDepartures;

        calculateButtonStates(buttonFrameStates_, departures, triggerArrivals_);
        for (auto const &buttonFrameState : buttonFrameStates_) {
            //protoButton:calculateGlitz(tempStore.forwardsGlitz, tempStore.reverseGlitz, tempStore.persistentGlitz)
            buttonFrameState.fillTrigger(outputTriggers_);
        }
        for (auto const &mutatorFrameState : mutatorFrameStates_) {
            //protoMutator:calculateGlitz(tempStore.forwardsGlitz, tempStore.reverseGlitz, tempStore.persistentGlitz)
            mutatorFrameState.fillTrigger(outputTriggers_);
        }
        mp::std::vector<TriggerData> triggerVector(pool_);
        boost::push_back(triggerVector, outputTriggers_ | boost::adaptors::transformed([](auto &&v) {return TriggerData{v.first, std::move(v.second)};}));
        return {
            calculateActualTriggerDepartures(triggerVector, triggerOffsetsAndDefaults_, currentFrame),
            forwardsGlitz,
            reverseGlitz,
            additionalGlitzPersisters,
            additionalBoxDepartures };
    }


    bool SimpleConfiguredTriggerFrameState::shouldArrive(Guy const &potentialArriver) { return true; }
    bool SimpleConfiguredTriggerFrameState::shouldArrive(Box const &potentialArriver) { return true; }

    bool SimpleConfiguredTriggerFrameState::shouldPort(
        int responsiblePortalIndex,
        Guy const &potentialPorter,
        bool porterActionedPortal) { return true; }
    bool SimpleConfiguredTriggerFrameState::shouldPort(
        int responsiblePortalIndex,
        Box const &potentialPorter,
        bool porterActionedPortal) { return true; }

    template<typename T>
    boost::optional<T> mutateObjectGeneric(
        mp::std::vector<int> const &responsibleMutatorIndices,
        T const &objectToManipulate,
        mp::std::vector<MutatorFrameState *> const &activeMutators)
    {
        boost::optional<T> dynamicObject{objectToManipulate};
        for (auto i : responsibleMutatorIndices) {
            assert(i >= 0 && i < activeMutators.size());
            if (!dynamicObject) {
                dynamicObject = activeMutators[i]->effect(*dynamicObject);
            }
            else {
                break;
            }
        }
        return dynamicObject;
    }

    boost::optional<Guy> SimpleConfiguredTriggerFrameState::mutateObject(
        mp::std::vector<int> const &responsibleMutatorIndices,
        Guy const &objectToManipulate)
    {
        return mutateObjectGeneric(responsibleMutatorIndices, objectToManipulate, activeMutators_);
    }
    boost::optional<Box> SimpleConfiguredTriggerFrameState::mutateObject(
        mp::std::vector<int> const &responsibleMutatorIndices,
        Box const &objectToManipulate)
    {
        return mutateObjectGeneric(responsibleMutatorIndices, objectToManipulate, activeMutators_);
    }

    SimpleConfiguredTriggerFrameState::~SimpleConfiguredTriggerFrameState() noexcept {}
}
