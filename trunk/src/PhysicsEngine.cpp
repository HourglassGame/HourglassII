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

#include "Foreach.h"

#include <cassert>

namespace hg {
PhysicsEngine::PhysicsEngine(
    Environment const& env,
    TriggerSystem const& triggerSystem) :
        env_(env),
        triggerSystem_(triggerSystem)
{
}

static GuyOutputInfo constructGuyOutputInfo(Guy const& guy) {
    return GuyOutputInfo(guy.getTimeDirection(), guy.getPickups());
}

struct NextPersister : std::unary_function<GlitzPersister const&, ObjectAndTime<GlitzPersister, Frame*> >
{
    NextPersister(Frame* frame): frame_(frame) {}
    ObjectAndTime<GlitzPersister, Frame*> operator()(GlitzPersister const& persister) const {
        return persister.runStep(frame_);
    }
    private:
    Frame* frame_;
};

PhysicsEngine::PhysicsReturnT PhysicsEngine::executeFrame(
    ObjectPtrList<Normal> const& arrivals,
    Frame* frame,
    std::vector<InputList> const& playerInput,
    OperationInterrupter& interrupter) const
{
    TriggerFrameState triggerFrameState(triggerSystem_.getFrameState(interrupter));

    //{extra boxes, collision, death, portal, pickup, arrival location}
    PhysicsAffectingStuff const physicsTriggerStuff(
        triggerFrameState.calculatePhysicsAffectingStuff(frame, arrivals.getList<TriggerData>()));

    mt::std::vector<ObjectAndTime<Box, Frame*> >::type nextBox;
    mt::std::vector<char>::type nextBoxNormalDeparture;

    mt::std::vector<Glitz>::type forwardsGlitz;
    mt::std::vector<Glitz>::type reverseGlitz;

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
        frame);

    bool currentPlayerFrame(currentPlayerInArrivals(arrivals.getList<Guy>(), playerInput.size()));
    bool nextPlayerFrame(false);
    bool winFrame(false);

    mt::std::vector<GuyOutputInfo>::type guyInfo;
    boost::push_back(
        guyInfo,
        arrivals.getList<Guy>()
            | boost::adaptors::transformed(constructGuyOutputInfo));

    mt::std::vector<ObjectAndTime<Guy, Frame*> >::type nextGuy;
    mt::std::vector<GlitzPersister>::type persistentGlitz;
    boost::push_back(persistentGlitz, arrivals.getList<GlitzPersister>());

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
        physicsTriggerStuff.collisions,
        physicsTriggerStuff.portals,
        physicsTriggerStuff.arrivalLocations,
        physicsTriggerStuff.mutators,
        triggerFrameState,
        GuyGlitzAdder(forwardsGlitz, reverseGlitz),
        nextPlayerFrame,
        winFrame);
		
	makeBoxGlitzListForNormalDepartures(
		nextBox,
		nextBoxNormalDeparture,
		BoxGlitzAdder(forwardsGlitz, reverseGlitz));

    buildDepartures(
        nextBox,
        nextGuy,
        newDepartures,
        frame);

    //Sort all object lists before returning to other code. They must be sorted for comparisons to work correctly.
    boost::for_each(newDepartures | boost::adaptors::map_values, SortObjectList());
    typedef mt::std::map<
                Frame*,
                mt::std::vector<TriggerData>::type>::type triggerDepartures_t;

    TriggerFrameState::DepartureInformation triggerSystemDepartureInformation(
        triggerFrameState.getDepartureInformation(
            newDepartures,
            frame));
    typedef triggerDepartures_t::value_type triggerDeparture_t;
    foreach (triggerDeparture_t const& triggerDeparture, triggerSystemDepartureInformation.triggerDepartures) {
        //Should probably move triggerDeparture.second into newDepartures, rather than copy it.
        newDepartures[triggerDeparture.first].addRange(triggerDeparture.second);
    }

    // add extra boxes to newDepartures
    buildDeparturesForComplexEntities(triggerSystemDepartureInformation.additionalBoxDepartures, newDepartures);

    foreach (GlitzPersister const& persister, persistentGlitz) {
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
    return PhysicsReturnT(
        newDepartures,
        FrameView(forwardsGlitz, reverseGlitz, guyInfo),
        currentPlayerFrame,
        nextPlayerFrame,
        winFrame);
}
} //namespace hg
