#include "SimpleConfiguredTriggerSystem.h"
#include "CommonTriggerCode.h"
#include "Maths.h"
#include "RectangleGlitz.h"
#include "ImageGlitz.h"
#include "AudioGlitz.h"
#include "TextGlitz.h"
#include "LuaUtilities.h"
#include "LuaError.h"
#include "LuaStackManager.h"
#include "LuaSandbox.h"
#include "mt/std/memory"
#include "lua\lauxlib.h"
#include <cmath>

namespace hg {
    //TODO: Move this to a central location
    unsigned asPackedColour(int const r, int const g, int const b) { 
        return r << 24 | g << 16 | b << 8;
    }

    //TODO: Remove this overload!
    std::tuple<Glitz, Glitz> calculateBidirectionalGlitz(
        int const layer,
        int const x, int const y, int const xspeed, int const yspeed, int const width, int const height, TimeDirection const timeDirection,
        unsigned const forwardsColour,
        unsigned const reverseColour)
    {
        Glitz sameDirectionGlitz(mt::std::make_unique<RectangleGlitz>(
            layer,
            x - xspeed, y - yspeed,
            width, height,
            forwardsColour
        ));
        Glitz oppositeDirectionGlitz(mt::std::make_unique<RectangleGlitz>(
            layer,
            x - xspeed, y - yspeed,
            width, height,
            reverseColour
        ));
        if (timeDirection == TimeDirection::FORWARDS) {
            return {std::move(sameDirectionGlitz), std::move(oppositeDirectionGlitz)};
        }
        else {
            return {std::move(oppositeDirectionGlitz), std::move(sameDirectionGlitz)};
        }
    }

    std::tuple<Glitz, Glitz> calculateBidirectionalGlitz(
        int const layer,
        DynamicArea const &dynamicArea,
        unsigned const forwardsColour,
        unsigned const reverseColour)
    {
        return calculateBidirectionalGlitz(
            layer,
            dynamicArea.x, dynamicArea.y, dynamicArea.xspeed, dynamicArea.yspeed, dynamicArea.width, dynamicArea.height, dynamicArea.timeDirection,
            forwardsColour,
            reverseColour);
    }

    std::tuple<Glitz, Glitz> calculateButtonGlitz(DynamicArea const &buttonArea, bool const buttonState) {
        auto const colour = buttonState ? asPackedColour(150, 255, 150) : asPackedColour(255, 150, 150);

        return calculateBidirectionalGlitz(400, buttonArea, colour, colour);
    }
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
                if (velocity * direction > 0) {
                    velocity -= direction*deceleration;
                    if (velocity * direction < 0) {
                        velocity = 0;
                    }
                }
                else if (
                    abs(position - desiredPosition)
                    >
                    (pow(velocity - direction * acceleration, 2) * 3. / (2. * deceleration)))
                {
                    velocity -= direction * acceleration;
                }
                else if (abs(position - desiredPosition) > pow(velocity, 2)*3./(2.*deceleration))
                {
                    velocity = -direction * sqrt(abs(position-desiredPosition)*deceleration*2./3.);
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
            assert(3 < lastStateTrigger.size());
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
        PortalArea retPortal(
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
        );
        
        //TODO: if protoPortal.triggerFunction, call triggerFunction!

        return {
            retPortal, true
        };
    }

    mt::std::string formatTime(int const frames) {
        auto const framesPerSecond = 60; //TODO: Move this to a central location?
        std::stringstream ss; //TODO: Use appropriate allocator here!
        
        if (frames%framesPerSecond == 0) {
            ss << floor(frames/static_cast<double>(framesPerSecond)) << "s";
        }
        else {
            double const seconds = frames/static_cast<double>(framesPerSecond);
            ss << std::setprecision(std::ceil(std::log10(std::abs(seconds))));
            ss << frames/static_cast<double>(framesPerSecond) << "s";
        }
        //TODO: Use appropriate allocator here too!!
        auto s = ss.str();
        return {s.begin(), s.end()};
    }
    void calculatePortalGlitz(
        PortalArea const &portal,
        mt::std::vector<Glitz> &forwardsGlitz,
        mt::std::vector<Glitz> &reverseGlitz,
        bool const active)
    {
        #if 0
        local colour
        if active then
            colour = {r = 120, g = 120, b = 120}, {r = 120, g = 120, b = 120}
        else
            colour = {r = 70, g = 70, b = 70}, {r = 70, g = 70, b = 70}
        end

        local forGlitz, revGlitz = calculateBidirectionalGlitz(350, portal, colour, colour)
        
        -- Text does not support changing time directions
        local text
        if portal.winner then
            text = active and "Win" or "Inactive"
        elseif portal.relativeTime and portal.timeDestination > 0 then
            text = "+" .. formatTime(portal.timeDestination)
        else
            text = formatTime(portal.timeDestination)
        end
        
        local textGlitz = {
            type = "text",
            x = portal.x+portal.width/2-1600,
            y = portal.y-2400,
            text = text,
            size = 2000,
            layer = 440,
            colour = {r = 0, g = 0, b = 0},
        }
        
        table.insert(forwardsGlitz, forGlitz)
        table.insert(reverseGlitz, revGlitz)
        table.insert(forwardsGlitz, textGlitz)
        table.insert(reverseGlitz, textGlitz)
        #endif

        auto const colour = active ? asPackedColour(120,120,120) : asPackedColour(70,70,70);
        //TODO: asDynamicArea(portal)?
        auto const [forGlitz, revGlitz] = calculateBidirectionalGlitz(350, portal.getX(), portal.getY(), portal.getXspeed(), portal.getYspeed(), portal.getWidth(), portal.getHeight(), portal.getTimeDirection(), colour, colour);
        
        mt::std::string text;
        if (portal.getWinner()) {
            text = active ? "Win" : "Inactive";
        }
        else if (portal.getRelativeTime() and portal.getTimeDestination() > 0) {
            text = "+" + formatTime(portal.getTimeDestination());
        }
        else {
            text = formatTime(portal.getTimeDestination());
        }

        /*
        int layer,
        mt::std::string text,
        int x, int y,
        int size,
        unsigned colour
        */
        auto textGlitz = Glitz(mt::std::make_unique<TextGlitz>(
            440,
            text,
            portal.getX()+portal.getWidth()/2-1600,
            portal.getY()-2400,
            2000,
            asPackedColour(0,0,0)
        ));
        forwardsGlitz.push_back(forGlitz);
        reverseGlitz.push_back(revGlitz);

        forwardsGlitz.push_back(textGlitz);
        reverseGlitz.push_back(textGlitz);
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
                calculatePortalGlitz(portal, forwardsGlitz, reverseGlitz, active);//TODO
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
        mp::std::vector<MutatorFrameStateImpl *> &activeMutators,
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
        buttonFrameStates_.reserve(protoButtons.size());
        for(auto &&a : protoButtons) buttonFrameStates_.push_back(a.getFrameState(pool_));
        
        mutatorFrameStates_.reserve(protoButtons.size());
        for(auto &&a : protoMutators) mutatorFrameStates_.push_back(a.getFrameState(pool_));
    }


    std::tuple<Glitz, Glitz> calculateCollisionGlitz(Collision const &collision) {
        return calculateBidirectionalGlitz(
            300,
            collision.getX(), collision.getY(), collision.getXspeed(), collision.getYspeed(), collision.getWidth(), collision.getHeight(), collision.getTimeDirection(),
            asPackedColour(50, 0, 0), asPackedColour(0, 0, 50));
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

        for (auto &&collision : retv.collisions) {
            auto [forwardsGlitz, reverseGlitz] = calculateCollisionGlitz(collision);
            forwardsGlitz_.push_back(std::move(forwardsGlitz));
            reverseGlitz_.push_back(std::move(reverseGlitz));
        }

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
        //mt::std::vector<Glitz> forwardsGlitz;
        //mt::std::vector<Glitz> reverseGlitz;
        mt::std::vector<GlitzPersister> additionalGlitzPersisters;
        mt::std::vector<ObjectAndTime<Box, Frame *>> additionalBoxDepartures;

        calculateButtonStates(buttonFrameStates_, departures, triggerArrivals_);
        for (auto const &buttonFrameState : buttonFrameStates_) {
            buttonFrameState.calculateGlitz(forwardsGlitz_, reverseGlitz_, additionalGlitzPersisters);
            buttonFrameState.fillTrigger(outputTriggers_);
        }
        for (auto const &mutatorFrameState : mutatorFrameStates_) {
            mutatorFrameState.calculateGlitz(forwardsGlitz_, reverseGlitz_, additionalGlitzPersisters);
            mutatorFrameState.fillTrigger(outputTriggers_);
        }
        mp::std::vector<TriggerData> triggerVector(pool_);
        boost::push_back(triggerVector, outputTriggers_ | boost::adaptors::transformed([](auto &&v) {return TriggerData{v.first, std::move(v.second)};}));
        return {
            calculateActualTriggerDepartures(triggerVector, triggerOffsetsAndDefaults_, currentFrame),
            forwardsGlitz_,
            reverseGlitz_,
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
        mp::std::vector<MutatorFrameStateImpl *> const &activeMutators)
    {
        boost::optional<T> dynamicObject{objectToManipulate};
        for (auto i : responsibleMutatorIndices) {
            assert(i >= 0 && i < activeMutators.size());
            dynamicObject = activeMutators[i]->effect(*dynamicObject);

            if (!dynamicObject) {
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


    template<>
    Attachment to<Attachment>(lua_State *L, int index) {
        try {
            if (!lua_istable(L, index)) {
                BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("Attachments must be tables"));
            }

            lua_getfield(L, index, "platform");
            bool hasPlatform(!lua_isnil(L, -1));
            int platform(hasPlatform ?lua_index_to_C_index(to<int>(L)):0);
            lua_pop(L, 1);

            int xOffset(readField<int>(L, "xOffset", index));
            int yOffset(readField<int>(L, "yOffset", index));
            return {
                hasPlatform,
                platform,
                xOffset,
                yOffset
            };
        }
        catch (LuaError &e) {
            add_semantic_callstack_info(e, "to<Attachment>");
            throw;
        }
    }

    ProtoPortal toProtoPortal(lua_State *L, std::size_t arrivalLocationsSize)
    {
        LuaStackManager stack_manager(L);
        if (!lua_istable(L, -1)) {
            BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("a protoPortal must be a table"));
        }
        #if 0
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
        #endif
        Attachment const attachment(readField<Attachment>(L, "attachment"));
        int const index(lua_index_to_C_index(readField<int>(L, "index")));
        int const width(readField<int>(L, "width"));
        int const height(readField<int>(L, "height"));
        int const collisionOverlap(readField<int>(L, "collisionOverlap"));
        TimeDirection const timeDirection(readField<TimeDirection>(L, "timeDirection"));
        int destinationIndex(-1);
        luaL_checkstack(L, 1, nullptr);
        lua_getfield(L, -1, "destinationIndex");
        if (!lua_isnil(L, -1)) {
            destinationIndex = lua_index_to_C_index(to<int>(L));
            if (!(destinationIndex >= 0 && static_cast<std::size_t>(destinationIndex) < arrivalLocationsSize))
            {
                BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("destinationIndex must be nil, or in the range [1, arrivalLocationsSize+1)"));
            }
        }
        lua_pop(L, 1);
        int const xDestination(readField<int>(L, "xDestination"));
        int const yDestination(readField<int>(L, "yDestination"));
        bool const relativeTime(readField<bool>(L, "relativeTime"));
        int const timeDestination(readField<int>(L, "timeDestination"));
        bool const relativeDirection(readFieldWithDefault<bool>(L, "relativeDirection", -1, true));
        TimeDirection const destinationDirection(readFieldWithDefault<TimeDirection>(L, "destinationDirection", -1, TimeDirection::FORWARDS));
        
        //TODO: Centralise this logic?
        lua_getfield(L, -1, "illegalDestination");
        int illegalDestination;
        if (lua_isnumber(L, -1)) {
            illegalDestination = lua_index_to_C_index(static_cast<int>(std::round(lua_tonumber(L, -1))));
            if (illegalDestination < 0)
            {
                BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("Proto Portal's illegalDestination must be >= 1"));
            }
        }
        else {
            if (!lua_isnil(L, -1))
            {
                BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("Proto Portal's illegalDestination must be an integer, or nil."));
            }
            illegalDestination = -1;
        }
        lua_pop(L, 1);

        bool const fallable(readField<bool>(L, "fallable"));
        bool const isLaser(readFieldWithDefault<bool>(L, "isLaser", -1, false));
        int const xaim(isLaser?readField<int>(L, "xaim"):0);
        int const yaim(isLaser?readField<int>(L, "yaim"):0);
        bool const winner(readField<bool>(L, "winner"));
        return {
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
        };
    }

    ProtoMutator toProtoPickup(
        lua_State * const L,
        std::vector<std::pair<int,std::vector<int>>> const &triggerOffsetsAndDefaults)
    {
        TimeDirection const timeDirection(readField<TimeDirection>(L, "timeDirection"));
        Attachment const attachment(readField<Attachment>(L, "attachment"));
        int const width(readField<int>(L, "width"));
        int const height(readField<int>(L, "height"));
        Ability const pickupType(readField<Ability>(L, "pickupType"));
        int const pickupNumber(readFieldWithDefault<int>(L, "pickupNumber", -1, 1));
        int const triggerID(lua_index_to_C_index(readField<int>(L, "triggerID")));
        assert(triggerID < triggerOffsetsAndDefaults.size());
        assert(0 < triggerOffsetsAndDefaults[triggerID].second.size());
        return ProtoMutator(mt::std::make_unique<ProtoPickupImpl>(
            timeDirection,
            attachment,
            width,
            height,
            pickupType,
            pickupNumber,
            triggerID
        ));
    }

    ProtoMutator toProtoSpikes(
        lua_State * const L)
    {
        TimeDirection const timeDirection(readField<TimeDirection>(L, "timeDirection"));
        Attachment const attachment(readField<Attachment>(L, "attachment"));
        int const width(readField<int>(L, "width"));
        int const height(readField<int>(L, "height"));
        return ProtoMutator(mt::std::make_unique<ProtoSpikesImpl>(
            timeDirection,
            attachment,
            width,
            height
        ));
    }

    ProtoMutator toProtoMutator(
        lua_State * const L,
        std::vector<std::pair<int,std::vector<int>>> const &triggerOffsetsAndDefaults) {
        std::string const type(readField<std::string>(L, "type"));
        if (type == "pickup") {
            return toProtoPickup(L, triggerOffsetsAndDefaults);
        }
        else if (type == "spikes") {
            return toProtoSpikes(L);
        }
        else {
            assert(false);
        }
    }

    
    template<>
    AxisCollisionDestination to<AxisCollisionDestination>(lua_State *L, int index) {
        try {
            if (!lua_istable(L, index)) {
                BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("AxisCollisionDestinations must be tables"));
            }

            return {
                readField<int>(L, "desiredPosition", index),
                readField<int>(L, "acceleration", index),
                readField<int>(L, "deceleration", index),
                readField<int>(L, "maxSpeed", index)
            };
        }
        catch (LuaError &e) {
            add_semantic_callstack_info(e, "to<AxisCollisionDestination>");
            throw;
        }
    }

    template<>
    CollisionDestination to<CollisionDestination>(lua_State *L, int index) {
        try {
            if (!lua_istable(L, index)) {
                BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("CollisionDestinations must be tables"));
            }

            return {
                readField<AxisCollisionDestination>(L, "xDestination", index),
                readField<AxisCollisionDestination>(L, "yDestination", index)
            };
        }
        catch (LuaError &e) {
            add_semantic_callstack_info(e, "to<CollisionDestination>");
            throw;
        }
    }

    ProtoCollision toProtoCollision(
        lua_State * const L,
        std::vector<std::pair<int,std::vector<int>>> const &triggerOffsetsAndDefaults)
    {

        TimeDirection const timeDirection(readField<TimeDirection>(L, "timeDirection"));
        int const width(readField<int>(L, "width"));
        int const height(readField<int>(L, "height"));


        lua_getfield(L, -1, "destinations");
        assert(lua_istable(L, -1));
        CollisionDestination const onDestination(readField<CollisionDestination>(L, "onDestination"));
        CollisionDestination const offDestination(readField<CollisionDestination>(L, "offDestination"));
        lua_pop(L, 1);

        lua_getfield(L, -1, "buttonTriggerID");
        bool const hasButtonTriggerID(!lua_isnil(L, -1));
        int buttonTriggerID(hasButtonTriggerID ?lua_index_to_C_index(to<int>(L)):0);
        lua_pop(L, 1);
        int const lastStateTriggerID(lua_index_to_C_index(readField<int>(L, "lastStateTriggerID")));

        assert(lastStateTriggerID < triggerOffsetsAndDefaults.size());
        assert(3 < triggerOffsetsAndDefaults[lastStateTriggerID].second.size());//Must have room for [x, y, xspeed, yspeed]

        return {
            timeDirection,
            width,
            height,
            onDestination,
            offDestination,
            hasButtonTriggerID,
            buttonTriggerID,
            lastStateTriggerID
        };
    }

    ProtoButton toProtoMomentarySwitch(
        lua_State * const L,
        std::vector<std::pair<int,std::vector<int>>> const &triggerOffsetsAndDefaults)
    {
        TimeDirection const timeDirection(readField<TimeDirection>(L, "timeDirection"));
        Attachment const attachment(readField<Attachment>(L, "attachment"));
        int const width(readField<int>(L, "width"));
        int const height(readField<int>(L, "height"));
        int const triggerID(lua_index_to_C_index(readField<int>(L, "triggerID")));
        int const stateTriggerID(lua_index_to_C_index(readFieldWithDefault<int>(L, "stateTriggerID", -1, C_index_to_lua_index(triggerID))));

        assert(triggerID < triggerOffsetsAndDefaults.size());
        assert(0 < triggerOffsetsAndDefaults[triggerID].second.size());

        assert(stateTriggerID < triggerOffsetsAndDefaults.size());
        assert(0 < triggerOffsetsAndDefaults[stateTriggerID].second.size());

        std::vector<int> extraTriggerIDs;
        lua_getfield(L, -1, "extraTriggerIDs");
        if (!lua_isnil(L, -1)) {
            assert(lua_istable(L, -1));

            lua_len(L, -1);
            lua_Integer const tablelen(lua_tointeger(L, -1));
            lua_pop(L, 1);
            extraTriggerIDs.reserve(tablelen);

            for (lua_Integer i(0), end(tablelen); i != end; ++i) {
                lua_rawgeti(L, -1, i + 1);
                extraTriggerIDs.push_back(to<int>(L, -1));
                lua_pop(L, 1);
            }
        }
        lua_pop(L, 1);

        return ProtoButton(mt::std::make_unique<ProtoMomentarySwitchImpl>(
            timeDirection,
            attachment,
            width,
            height,
            triggerID,
            stateTriggerID,
            std::move(extraTriggerIDs)
        ));
    }

    ProtoButton toProtoStickySwitch(
        lua_State * const L,
        std::vector<std::pair<int,std::vector<int>>> const &triggerOffsetsAndDefaults)
    {

        //TODO: extract this into shared function with toProtoMomentarySwitch

        TimeDirection const timeDirection(readField<TimeDirection>(L, "timeDirection"));
        Attachment const attachment(readField<Attachment>(L, "attachment"));
        int const width(readField<int>(L, "width"));
        int const height(readField<int>(L, "height"));
        int const triggerID(lua_index_to_C_index(readField<int>(L, "triggerID")));
        int const stateTriggerID(lua_index_to_C_index(readFieldWithDefault<int>(L, "stateTriggerID", -1, C_index_to_lua_index(triggerID))));

        assert(triggerID < triggerOffsetsAndDefaults.size());
        assert(0 < triggerOffsetsAndDefaults[triggerID].second.size());

        assert(stateTriggerID < triggerOffsetsAndDefaults.size());
        assert(0 < triggerOffsetsAndDefaults[stateTriggerID].second.size());

        std::vector<int> extraTriggerIDs;
        lua_getfield(L, -1, "extraTriggerIDs");
        if (!lua_isnil(L, -1)) {
            assert(lua_istable(L, -1));

            lua_len(L, -1);
            lua_Integer const tablelen(lua_tointeger(L, -1));
            lua_pop(L, 1);
            extraTriggerIDs.reserve(tablelen);

            for (lua_Integer i(0), end(tablelen); i != end; ++i) {
                lua_rawgeti(L, -1, i + 1);
                extraTriggerIDs.push_back(to<int>(L, -1));
                lua_pop(L, 1);
            }
        }
        lua_pop(L, 1);

        return ProtoButton(mt::std::make_unique<ProtoStickySwitchImpl>(
            timeDirection,
            attachment,
            width,
            height,
            triggerID,
            stateTriggerID,
            std::move(extraTriggerIDs)
        ));
    }

    ProtoButton toProtoButton(
        lua_State * const L,
        std::vector<std::pair<int,std::vector<int>>> const &triggerOffsetsAndDefaults)
    {
        std::string const type(readField<std::string>(L, "type"));
        if (type == "momentarySwitch") {
            return toProtoMomentarySwitch(L, triggerOffsetsAndDefaults);
        }
        else if (type == "stickySwitch") {
            return toProtoStickySwitch(L, triggerOffsetsAndDefaults);
        }
        else {
            assert(false);
        }
    }

    SimpleConfiguredTriggerSystem::SimpleConfiguredTriggerSystem(
            std::vector<char> const &mainChunk,
            std::vector<LuaModule> const &extraChunks,
            std::vector<
                std::pair<
                    int,
                    std::vector<int>
                >
            > triggerOffsetsAndDefaults,
            std::size_t arrivalLocationsSize)
        :
        protoPortals_(),
        protoCollisions_(),
        protoMutators_(),
        protoButtons_(),
        triggerOffsetsAndDefaults_(std::move(triggerOffsetsAndDefaults)),
        arrivalLocationsSize_(arrivalLocationsSize)
    {
        //#if 0
        LuaState triggerSystemGenerator((LuaState::new_state_t()));

        lua_State *const L(triggerSystemGenerator.ptr);
        //The use of the split sandboxing system isn't stricly necessary here,
        //since this is only ever called once; but it's easier than writing a whole new non-split
        //sandboxing system.
        loadSandboxedLibraries(L);
        pushFunctionFromVector(L, mainChunk, "triggerSystem");
        setUpPreloadResetFunction(L, extraChunks);

        //LuaInterruptionHandle h(makeInterruptable(L, interrupter));//TODO

        //LuaStackManager stackSaver(L); ?
        //checkstack(L, 1);
        restoreGlobals(L);

        lua_call(L, 0, 0);

        LuaStackManager stackSaver(L);
        //push function to call
        luaL_checkstack(L, 1, nullptr);
        lua_getglobal(L, "calculatePhysicsAffectingStuff");

        //TODO: Extract this 'Read table' code into a function
        //read 'protoPortals' table
        luaL_checkstack(L, 1, nullptr);
        lua_getfield(L, -1, "protoPortals");
        if (!lua_isnil(L, -1)) {
            if (!lua_istable(L, -1)) {
                BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("protoPortals must be a table"));
            }
            for (std::size_t i(1), end(lua_rawlen(L, -1)); i <= end; ++i) {
                luaL_checkstack(L, 1, nullptr);
                lua_pushinteger(L, i);
                lua_gettable(L, -2);
                protoPortals_.push_back(toProtoPortal(L, arrivalLocationsSize_));
                lua_pop(L, 1);
            }
        }
        lua_pop(L, 1);


        lua_getfield(L, -1, "protoMutators");
        if (!lua_isnil(L, -1)) {
            if (!lua_istable(L, -1)) {
                BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("protoMutators must be a table"));
            }
            for (std::size_t i(1), end(lua_rawlen(L, -1)); i <= end; ++i) {
                luaL_checkstack(L, 1, nullptr);
                lua_pushinteger(L, i);
                lua_gettable(L, -2);
                protoMutators_.push_back(toProtoMutator(L, triggerOffsetsAndDefaults_));
                lua_pop(L, 1);
            }
        }
        lua_pop(L, 1);


        lua_getfield(L, -1, "protoCollisions");
        if (!lua_isnil(L, -1)) {
            if (!lua_istable(L, -1)) {
                BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("protoCollisions must be a table"));
            }
            for (std::size_t i(1), end(lua_rawlen(L, -1)); i <= end; ++i) {
                luaL_checkstack(L, 1, nullptr);
                lua_pushinteger(L, i);
                lua_gettable(L, -2);
                protoCollisions_.push_back(toProtoCollision(L, triggerOffsetsAndDefaults_));
                lua_pop(L, 1);
            }
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "protoButtons");
        if (!lua_isnil(L, -1)) {
            if (!lua_istable(L, -1)) {
                BOOST_THROW_EXCEPTION(LuaInterfaceError() << basic_error_message_info("protoButtons must be a table"));
            }
            for (std::size_t i(1), end(lua_rawlen(L, -1)); i <= end; ++i) {
                luaL_checkstack(L, 1, nullptr);
                lua_pushinteger(L, i);
                lua_gettable(L, -2);
                protoButtons_.push_back(toProtoButton(L, triggerOffsetsAndDefaults_));
                lua_pop(L, 1);
            }
        }
        lua_pop(L, 1);
        //TODO...
        ;

        //Also
        //lua_getglobal mutators, shouldport, shouldarrive, etc

        //#endif
    }
    void PickupFrameStateImpl::calculateGlitz(
            mt::std::vector<Glitz> &forwardsGlitz,
            mt::std::vector<Glitz> &reverseGlitz,
            mt::std::vector<GlitzPersister> &persistentGlitz) const
    {
        //assert(false);//TODO
        assert(proto);

        static std::map<Ability, mt::std::string> const pickupGlitzNameMap{
            {Ability::TIME_JUMP, "global.time_jump"},
            {Ability::TIME_REVERSE, "global.time_reverse"},
            {Ability::TIME_GUN, "global.time_gun"},
            {Ability::TIME_PAUSE, "global.time_pause"}
        };

        assert(proto->pickupType != Ability::NO_ABILITY);
        if (active) {
            forwardsGlitz.push_back(
                Glitz(mt::std::make_unique<ImageGlitz>(
                    430,
                    pickupGlitzNameMap.find(proto->pickupType)->second,//TODO: Avoid undefined behaviour on unfound ability
                    x_, y_,
                    proto->width, proto->height
                ))
            );
            reverseGlitz.push_back(
                Glitz(mt::std::make_unique<ImageGlitz>(
                    430,
                    pickupGlitzNameMap.find(proto->pickupType)->second,//TODO: Avoid undefined behaviour on unfound ability
                    x_, y_,
                    proto->width, proto->height
                ))
            );
        }
        if (justTaken) {
            //TODO: Avoid possible memory leak here!
            persistentGlitz.push_back(
                GlitzPersister(mt::std::make_unique<AudioGlitzPersister>(
                    "global.pickup_pickup",
                    9,
                    proto->timeDirection
                )));
        }
        #if 0
        if active then
            local forwardsImageGlitz = {
                type = "image",
                layer = 430,
                key = pickupGlitzNameMap[proto.pickupType],
                x = PnV.x,
                y = PnV.y,
                width = proto.height,
                height = proto.width
            }

            local reverseImageGlitz = {
                type = "image",
                layer = 430,
                key = pickupGlitzNameMap[proto.pickupType],
                x = PnV.x,
                y = PnV.y,
                width = proto.height,
                height = proto.width
            }
            table.insert(forwardsGlitz, forwardsImageGlitz)
            table.insert(reverseGlitz, reverseImageGlitz)
        end
        if justTaken then
            table.insert(
                persistentGlitz,
                {type = "audio",
                    key = "global.pickup_pickup",
                    duration = 9,
                    timeDirection = proto.timeDirection})
        end
        #endif

    }
    void PickupFrameStateImpl::fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const {
        assert(proto);
        outputTriggers[proto->triggerID] = mt::std::vector<int>{active};
    }

    boost::optional<Guy> PickupFrameStateImpl::effect(Guy const &guy) {
        //assert(false);
        assert(proto);
        #if 0
        if not active then 
            return dynamicObject 
        end
        if dynamicObject.type ~= 'guy' then
            return dynamicObject 
        end
        dynamicObject.pickups[proto.pickupType] =
            dynamicObject.pickups[proto.pickupType] + (proto.pickupNumber or 1)
        active = false
        justTaken = true
        return dynamicObject
        #endif
        if (!active) return guy;
        active = false;
        justTaken = true;
        mt::std::map<Ability, int> pickups(guy.getPickups());
        pickups[proto->pickupType] += proto->pickupNumber;

        return Guy(
            guy.getIndex(),
            guy.getX(), guy.getY(),
            guy.getXspeed(), guy.getYspeed(),
            guy.getWidth(), guy.getHeight(),
            guy.getJumpSpeed(),

            guy.getIllegalPortal(),
            guy.getArrivalBasis(),
            guy.getSupported(),
            guy.getSupportedSpeed(),

            pickups,
            guy.getFacing(),

            guy.getBoxCarrying(),
            guy.getBoxCarrySize(),
            guy.getBoxCarryDirection(),

            guy.getTimeDirection(),
            guy.getTimePaused());
    }
    void PickupFrameStateImpl::addMutator(
            mp::std::vector<mp::std::vector<int>> const &triggerArrivals,
            mp::std::vector<Collision> const &collisions,
            mp::std::vector<MutatorArea> &mutators,
            mp::std::vector<MutatorFrameStateImpl *> &activeMutators
        )
    {
        #if 0
        if triggerArrivals[triggerID][1] == 1 then
            PnV = calculateButtonPositionAndVelocity(proto, collisions)
            mutators[#mutators+1] = {
                x = PnV.x, y = PnV.y,
                width = proto.width, height = proto.height,
                xspeed = PnV.xspeed, yspeed = PnV.yspeed,
                collisionOverlap = 0,
                timeDirection = proto.timeDirection
            }
            activeMutators[#activeMutators+1] = self
        else
            active = false
        end
        #endif
        assert(proto);
        assert(triggerArrivals.size() > proto->triggerID);
        assert(triggerArrivals[proto->triggerID].size() > 0);
        if (triggerArrivals[proto->triggerID][0]) {
            auto const [x, y, xspeed, yspeed] = snapAttachment(proto->timeDirection, proto->attachment, collisions);
            x_ = x;
            y_ = y;
            mutators.emplace_back(
                x, y,
                proto->width, proto->height,
                xspeed, yspeed,
                0,//collisionOverlap
                proto->timeDirection);
            activeMutators.emplace_back(this);
        }
        else {
            active = false;
        }

    }

    void SpikesFrameStateImpl::addMutator(
            mp::std::vector<mp::std::vector<int>> const &triggerArrivals,
            mp::std::vector<Collision> const &collisions,
            mp::std::vector<MutatorArea> &mutators,
            mp::std::vector<MutatorFrameStateImpl *> &activeMutators
        )
    {
        auto [x, y, xspeed, yspeed] = snapAttachment(proto->timeDirection, proto->attachment, collisions);
        x_ = x;
        y_ = y;
        xspeed_ = xspeed;
        yspeed_ = yspeed;
        mutators.emplace_back(
            x, y,
            proto->width, proto->height,
            xspeed, yspeed,
            1,//collisionOverlap; So that spikes just below the surface are not deadly
            proto->timeDirection);
        activeMutators.emplace_back(this);
    }

    void SpikesFrameStateImpl::calculateGlitz(
            mt::std::vector<Glitz> &forwardsGlitz,
            mt::std::vector<Glitz> &reverseGlitz,
            mt::std::vector<GlitzPersister> &persistentGlitz) const
    {
        assert(proto);
        {
            auto const colour = asPackedColour(255, 0, 0);
            auto const [forGlitz, revGlitz] = calculateBidirectionalGlitz(430, x_, y_, xspeed_, yspeed_, proto->width, proto->height, proto->timeDirection, colour, colour);
            forwardsGlitz.push_back(forGlitz);
            reverseGlitz.push_back(revGlitz);
        }
        for (auto &&death : deathGlitz) {
            auto const [forGlitz, revGlitz] = calculateBidirectionalGlitz(430, death.x, death.y, death.xspeed, death.yspeed, death.width, death.height, death.timeDirection, asPackedColour(255,0,0), asPackedColour(0, 255, 255));
            forwardsGlitz.push_back(forGlitz);
            reverseGlitz.push_back(revGlitz);
        }
    }

    void SpikesFrameStateImpl::fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const {
        //Do Nothing
    }
    boost::optional<Guy> SpikesFrameStateImpl::effect(Guy const &guy) {
        deathGlitz.push_back(
            //TODO: getDynamicArea(guy)?
            {
                guy.getX(),
                guy.getY(),
                guy.getXspeed(),
                guy.getYspeed(),
                guy.getWidth(),
                guy.getHeight(),
                guy.getTimeDirection()
            }
            
        );
        #if 0
        deathGlitz[#deathGlitz+1] = {
            x = dynamicObject.x,
            y = dynamicObject.y,
            width = dynamicObject.width,
            height = dynamicObject.height,
            timeDirection = dynamicObject.timeDirection
        }
        return nil
        #endif
        return {};
    }

    //TODO: Does this logic already exist somewhere in C++??
    bool temporalIntersectingExclusive(
        int const xa_raw, int const ya_raw, int const xspeeda, int const yspeeda, int const wa, int const ha, TimeDirection const timeDirectiona,
        int const xb_raw, int const yb_raw, int const xspeedb, int const yspeedb, int const wb, int const hb, TimeDirection const timeDirectionb 
    )
    {
        int const xa = timeDirectiona == TimeDirection::FORWARDS ? xa_raw : xa_raw - xspeeda;
        int const ya = timeDirectiona == TimeDirection::FORWARDS ? ya_raw : ya_raw - yspeeda;

        int const xb = timeDirectiona == TimeDirection::FORWARDS ? xb_raw : xb_raw - xspeedb;
        int const yb = timeDirectiona == TimeDirection::FORWARDS ? yb_raw : yb_raw - yspeedb;

        return
            (
                (xa < xb && xa + wa > xb)
                    ||
                (xb < xa && xb + wb > xa)
                    ||
                (xa == xb)
            )
                &&
            (
                (ya < yb && ya + ha > yb)
                    ||
                (yb < ya && yb + hb > ya)
                    ||
                (ya == yb)
            );
    }
    bool checkPressed(
        int const x, int const y, int const xspeed, int const yspeed, int const width, int const height, TimeDirection const timeDirection,
        mt::std::map<Frame*, ObjectList<Normal>> const &departures)
    {

        #if 0
        local function checkPressed(buttonArea, departures)
            for frame, objectList in pairs(departures) do
                for box in list_iter(objectList.boxes) do
                    if temporalIntersectingExclusive(buttonArea, box) then
                        return true
                    end
                end
                for guy in list_iter(objectList.guys) do
                    if temporalIntersectingExclusive(buttonArea, guy) then
                        return true
                    end
                end
            end
            return false
        end
        #endif

        for(auto &&[_, objectList]:departures) {
            (void)_;
            for (auto &&box : objectList.getList<Box>()) {
                if (temporalIntersectingExclusive(
                        x, y, xspeed, yspeed, width, height, timeDirection,
                        box.getX(), box.getY(), box.getXspeed(), box.getYspeed(), box.getWidth(), box.getHeight(), box.getTimeDirection()
                    ))
                {
                    return true;
                }
            }
            for (auto &&guy : objectList.getList<Guy>()) {
                if (temporalIntersectingExclusive(
                        x, y, xspeed, yspeed, width, height, timeDirection,
                        guy.getX(), guy.getY(), guy.getXspeed(), guy.getYspeed(), guy.getWidth(), guy.getHeight(), guy.getTimeDirection()
                    ))
                {
                    return true;
                }
            }
        }
        return false;
    }


    void MomentarySwitchFrameStateImpl::calcPnV(mp::std::vector<Collision> const &collisions)
    {
        std::tie(x, y, xspeed, yspeed) = snapAttachment(proto->timeDirection, proto->attachment, collisions);
    }
    void MomentarySwitchFrameStateImpl::updateState(
        mt::std::map<Frame*, ObjectList<Normal>> const &departures,
        mp::std::vector<mp::std::vector<int>> const &triggerArrivals)
    {
        assert(proto->stateTriggerID < triggerArrivals.size());
        assert(0 < triggerArrivals[proto->stateTriggerID].size());
        int const oldState = triggerArrivals[proto->stateTriggerID][0];
        state = std::min(
            checkPressed(x, y, xspeed, yspeed, proto->width, proto->height, proto->timeDirection, departures) ? oldState+1 : 0,
            2);
        justPressed = oldState == 0 && state > 0;
        justReleased = oldState > 0 && state == 0;
    }
    void MomentarySwitchFrameStateImpl::calculateGlitz(
            mt::std::vector<Glitz> &forwardsGlitz,
            mt::std::vector<Glitz> &reverseGlitz,
            mt::std::vector<GlitzPersister> &persistentGlitz) const
    {

        #if 0
        local forGlitz, revGlitz = calculateButtonGlitz(proto, PnV, state > 0)
        table.insert(forwardsGlitz, forGlitz)
        table.insert(reverseGlitz, revGlitz)
        if justPressed then
            table.insert(
                persistentGlitz,
                {type = "audio", key = "global.switch_push_down", duration = 10, timeDirection = proto.timeDirection})
        end
        if justReleased then
            table.insert(
                persistentGlitz,
                {type = "audio", key = "global.switch_push_up", duration = 10, timeDirection = proto.timeDirection})
        end
        #endif

        auto [forGlitz, revGlitz] = calculateButtonGlitz(
            DynamicArea{
               x,
               y,
               xspeed,
               yspeed,
               proto->width,
               proto->height,
               proto->timeDirection},
            state > 0);
        forwardsGlitz.push_back(std::move(forGlitz));
        reverseGlitz.push_back(std::move(revGlitz));

        if (justPressed) {
            persistentGlitz.push_back(
                GlitzPersister(mt::std::make_unique<AudioGlitzPersister>("global.switch_push_down", 10, proto->timeDirection))
            );
        }
        if (justReleased) {
            persistentGlitz.push_back(
                GlitzPersister(mt::std::make_unique<AudioGlitzPersister>("global.switch_push_up", 10, proto->timeDirection))
            );
        }
    }
    void MomentarySwitchFrameStateImpl::fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const {
        outputTriggers[proto->triggerID] = mt::std::vector<int>{state};
        outputTriggers[proto->stateTriggerID] = mt::std::vector<int>{state};
        for (auto extraTriggerID : proto->extraTriggerIDs) {
            outputTriggers[extraTriggerID] = mt::std::vector<int>{state};
        }
    }


    void StickySwitchFrameStateImpl::calcPnV(mp::std::vector<Collision> const &collisions)
    {
        std::tie(x, y, xspeed, yspeed) = snapAttachment(proto->timeDirection, proto->attachment, collisions);
    }
    void StickySwitchFrameStateImpl::updateState(
        mt::std::map<Frame*, ObjectList<Normal>> const &departures,
        mp::std::vector<mp::std::vector<int>> const &triggerArrivals)
    {
        assert(proto->stateTriggerID < triggerArrivals.size());
        assert(0 < triggerArrivals[proto->stateTriggerID].size());
        int const oldState = triggerArrivals[proto->stateTriggerID][0];
        state = std::min(
            oldState > 0 ?
              oldState+1
            : (checkPressed(x, y, xspeed, yspeed, proto->width, proto->height, proto->timeDirection, departures) ? 1 : 0),
            2);
        justPressed = oldState == 0 && state > 0;
    }
    void StickySwitchFrameStateImpl::calculateGlitz(
            mt::std::vector<Glitz> &forwardsGlitz,
            mt::std::vector<Glitz> &reverseGlitz,
            mt::std::vector<GlitzPersister> &persistentGlitz) const
    {
        auto [forGlitz, revGlitz] = calculateButtonGlitz(
            DynamicArea{
               x,
               y,
               xspeed,
               yspeed,
               proto->width,
               proto->height,
               proto->timeDirection},
            state > 0);
        forwardsGlitz.push_back(std::move(forGlitz));
        reverseGlitz.push_back(std::move(revGlitz));

        if (justPressed) {
            persistentGlitz.push_back(
                GlitzPersister(mt::std::make_unique<AudioGlitzPersister>("global.switch_push_down", 10, proto->timeDirection))
            );
        }
    }
    void StickySwitchFrameStateImpl::fillTrigger(mp::std::map<std::size_t, mt::std::vector<int>> &outputTriggers) const {
        outputTriggers[proto->triggerID] = mt::std::vector<int>{state};
        outputTriggers[proto->stateTriggerID] = mt::std::vector<int>{state};
        for (auto extraTriggerID : proto->extraTriggerIDs) {
            outputTriggers[extraTriggerID] = mt::std::vector<int>{state};
        }
    }
}
