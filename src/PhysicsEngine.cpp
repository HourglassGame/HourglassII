#include "PhysicsEngine.h"

#include "Frame.h"
#include "Universe.h"
#include "Geometry.h"
#include "PhysicsEngineUtilities.h"

#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/optional.hpp>
#include <boost/tuple/tuple.hpp>

#include "mt/std/map"
#include "mt/std/vector"

#include <cassert>

#include "AudioGlitz.h"

namespace hg {
PhysicsEngine::PhysicsEngine(
    Environment const &env,
    TriggerSystem const &triggerSystem) :
        env_(env),
        triggerSystem_(triggerSystem)
{
}
struct ConstructGuyOutputInfo {
    ConstructGuyOutputInfo(mt::std::vector<ArrivalLocation> const &arrivalLocations) :
        arrivalLocations(&arrivalLocations) {}
    GuyOutputInfo operator()(Guy const &guy) const {
        int x = guy.getX();
        int y = guy.getY();
        if (guy.getArrivalBasis() != -1) {
            x += (*arrivalLocations)[guy.getArrivalBasis()].getX();
            y += (*arrivalLocations)[guy.getArrivalBasis()].getY();
        }
        return GuyOutputInfo(
            guy.getIndex(), guy.getTimeDirection(), guy.getPickups(),
            guy.getBoxCarrying(), guy.getBoxCarryDirection(), x, y);
    }
    mt::std::vector<ArrivalLocation> const *arrivalLocations;
};
struct NextPersister
{
    NextPersister(Frame *frame): frame_(frame) {}
    ObjectAndTime<GlitzPersister, Frame *> operator()(GlitzPersister const &persister) const {
        return persister.runStep(frame_);
    }
    private:
    Frame *frame_;
};

PhysicsEngine::PhysicsReturnT PhysicsEngine::executeFrame(
    ObjectPtrList<Normal> const &arrivals,
    Frame *frame,
    std::vector<InputList> const &playerInput,
    OperationInterrupter &interrupter) const
{
    TriggerFrameState triggerFrameState(triggerSystem_.getFrameState(interrupter));

    //{extra boxes, collision, death, portal, pickup, arrival location}
    PhysicsAffectingStuff const physicsTriggerStuff(
        triggerFrameState.calculatePhysicsAffectingStuff(frame, arrivals.getList<TriggerData>()));

    mt::std::vector<ObjectAndTime<Box, Frame *> > nextBox;
    mt::std::vector<char> nextBoxNormalDeparture;

    mt::std::vector<Glitz> forwardsGlitz;
    mt::std::vector<Glitz> reverseGlitz;
    mt::std::vector<GlitzPersister> persistentGlitz;
    boost::push_back(persistentGlitz, arrivals.getList<GlitzPersister>());

    // boxes do their crazy wizz-bang collision algorithm
    boxCollisionAlogorithm(
        env_,
        arrivals.getList<Box>(),
        physicsTriggerStuff.additionalBoxes,
        nextBox,
        nextBoxNormalDeparture,
        physicsTriggerStuff.collisions,
        physicsTriggerStuff.portals,
        physicsTriggerStuff.arrivalLocations,
        physicsTriggerStuff.mutators,
        triggerFrameState,
        BoxGlitzAdder(forwardsGlitz, reverseGlitz, persistentGlitz),
        frame);

    bool currentPlayerFrame(currentPlayerInArrivals(arrivals.getList<Guy>(), playerInput.size()));
    bool nextPlayerFrame(false);
    bool winFrame(false);

    mt::std::vector<GuyOutputInfo> guyInfo;
    boost::push_back(
        guyInfo,
        arrivals.getList<Guy>()
            | boost::adaptors::transformed(ConstructGuyOutputInfo(physicsTriggerStuff.arrivalLocations)));

    mt::std::vector<ObjectAndTime<Guy, Frame*> > nextGuy;

    FrameDepartureT newDepartures;

    // guys simple collision algorithm
    guyStep(
        env_,
        arrivals.getList<Guy>(),
        frame,
        playerInput,
        nextGuy,
        nextBox,
        nextBoxNormalDeparture,
        arrivals.getList<Box>(),
        physicsTriggerStuff.collisions,
        physicsTriggerStuff.portals,
        physicsTriggerStuff.arrivalLocations,
        physicsTriggerStuff.mutators,
        triggerFrameState,
        GuyGlitzAdder(forwardsGlitz, reverseGlitz, persistentGlitz),
        nextPlayerFrame,
        winFrame);
    
    makeBoxGlitzListForNormalDepartures(
        nextBox,
        nextBoxNormalDeparture,
        BoxGlitzAdder(forwardsGlitz, reverseGlitz, persistentGlitz));

    buildDepartures(
        nextBox,
        nextGuy,
        newDepartures,
        frame);

    //Sort all object lists before returning to other code.
    //Object Lists are required to be sorted,
    //since they must be in a canonical form
    //for comparisons to work correctly (when updating departures).
    boost::for_each(newDepartures | boost::adaptors::map_values, SortObjectList());
    typedef mt::std::map<
                Frame *,
                mt::std::vector<TriggerData>> triggerDepartures_t;

    TriggerFrameState::DepartureInformation const triggerSystemDepartureInformation(
        triggerFrameState.getDepartureInformation(
            newDepartures,
            frame));

    for (auto const &triggerDeparture: triggerSystemDepartureInformation.triggerDepartures) {
        //Should probably move triggerDeparture.second into newDepartures, rather than copy it.
        newDepartures[triggerDeparture.first].addRange(triggerDeparture.second);
    }
    buildDeparturesForComplexEntities(triggerSystemDepartureInformation.additionalBoxDepartures, newDepartures);
    boost::push_back(persistentGlitz, triggerSystemDepartureInformation.additionalGlitzPersisters);

    for (GlitzPersister const &persister: persistentGlitz) {
        forwardsGlitz.push_back(persister.getForwardsGlitz());
        reverseGlitz.push_back(persister.getReverseGlitz());
    }

    boost::push_back(forwardsGlitz, triggerSystemDepartureInformation.forwardsGlitz);
    boost::push_back(reverseGlitz, triggerSystemDepartureInformation.reverseGlitz);

    buildDeparturesForComplexEntities(
        persistentGlitz | boost::adaptors::transformed(NextPersister(frame)), newDepartures);

    //also sort trigger departures. TODO: do this better (ie, don't re-sort non-trigger departures).
    boost::for_each(newDepartures | boost::adaptors::map_values, SortObjectList());

    // add data to departures
    return {
        newDepartures,
        FrameView(forwardsGlitz, reverseGlitz, guyInfo),
        currentPlayerFrame,
        nextPlayerFrame,
        winFrame};
}
} //namespace hg
