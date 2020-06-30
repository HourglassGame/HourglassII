#include "PhysicsEngine.h"

#include "PhysicsEngineUtilities.h"

#include "hg/TimeEngine/Frame.h"
#include "hg/TimeEngine/Universe.h"
#include "hg/TimeEngine/Glitz/AudioGlitz.h"

#include "hg/Util/memory_pool.h"
#include "hg/Util/maths.h"

#include "hg/mt/std/map"
#include "hg/mt/std/vector"
#include "hg/mp/std/vector"

#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/optional.hpp>

#include <cassert>
#include <tuple>

namespace hg {
PhysicsEngine::PhysicsEngine(
	Environment const &env,
	TriggerSystem const &triggerSystem) :
		env_(env),
		triggerSystem_(triggerSystem)

#if USE_POOL_ALLOCATOR
	,
		shared_pool_()
#endif
{
}
struct ConstructGuyOutputInfo {
	ConstructGuyOutputInfo(mp::std::vector<ArrivalLocation> const &arrivalLocations) :
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
	mp::std::vector<ArrivalLocation> const *arrivalLocations;
};
#if 0
struct NextPersister
{
	NextPersister(Frame *frame): frame_(frame) {}
	ObjectAndTime<GlitzPersister, Frame *> operator()(GlitzPersister const &persister) const {
		return persister.runStep(frame_);
	}
	private:
	Frame *frame_;
};
#endif
PhysicsEngine::PhysicsReturnT PhysicsEngine::executeFrame(
	ObjectPtrList<Normal> const &arrivals,
	Frame *frame,
	std::vector<GuyInput> const &playerInput,
	OperationInterrupter &interrupter) const
{
	//const std::size_t initialPoolSize{2<<5};
	//memory_pool<user_allocator_tbb_alloc> pool{initialPoolSize};
#if USE_POOL_ALLOCATOR
	auto &pool(shared_pool_->get());
	pool.consolidate_memory();
#else
	memory_pool<> pool;
#endif
	TriggerFrameState triggerFrameState(triggerSystem_.getFrameState(pool, interrupter));

	//{extra boxes, collision, death, portal, pickup, arrival location}
	PhysicsAffectingStuff const physicsTriggerStuff(
		triggerFrameState.calculatePhysicsAffectingStuff(frame, arrivals.getList<TriggerData>()));

	mp::std::vector<ObjectAndTime<Box, Frame *>> nextBox(pool);
	mp::std::vector<ObjectAndTime<Explosion, Frame *>> nextExplosion(pool);
	mp::std::vector<char> nextBoxNormalDeparture(pool);

	//TODO: Use frame-output memory pool
	mt::std::vector<Glitz> forwardsGlitz;
	mt::std::vector<Glitz> reverseGlitz;
	mp::std::vector<GlitzPersister> persistentGlitz(pool);
	boost::push_back(persistentGlitz, arrivals.getList<GlitzPersister>());

	// explosions
	mt::std::vector<ExplosionEffect> explosions;

	// boxes do their crazy wizz-bang collision algorithm
	boxCollisionAlgorithm(
		env_,
		arrivals.getList<Box>(),
		physicsTriggerStuff.additionalBoxes,
		nextBox,
		nextBoxNormalDeparture,
		arrivals.getList<Explosion>(),
		nextExplosion,
		physicsTriggerStuff.collisions,
		physicsTriggerStuff.portals,
		physicsTriggerStuff.arrivalLocations,
		physicsTriggerStuff.mutators,
		triggerFrameState,
		BoxGlitzAdder(forwardsGlitz, reverseGlitz, persistentGlitz),
		frame,
		pool);

	bool winFrame(false);

	mt::std::vector<GuyOutputInfo> guyInfo;
	boost::push_back(
		guyInfo,
		arrivals.getList<Guy>()
			| boost::adaptors::transformed(ConstructGuyOutputInfo(physicsTriggerStuff.arrivalLocations)));

	mp::std::vector<ObjectAndTime<Guy, Frame*>> nextGuy(pool);

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
		explosions,
		triggerFrameState,
		GuyGlitzAdder(forwardsGlitz, reverseGlitz, persistentGlitz),
		winFrame,
		pool);

	makeBoxGlitzListForNormalDepartures(
		nextBox,
		nextBoxNormalDeparture,
		BoxGlitzAdder(forwardsGlitz, reverseGlitz, persistentGlitz));

	mt::std::vector<std::tuple<std::size_t, Frame *>> guyDepartureFrames;
	buildDepartures(
		nextBox,
		nextExplosion,
		nextGuy,
		guyDepartureFrames,
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
			arrivals.getList<Explosion>(),
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
		persistentGlitz | boost::adaptors::transformed([=](GlitzPersister const& persister){return persister.runStep(frame);}), newDepartures);

	//also sort trigger departures. TODO: do this better (ie, don't re-sort non-trigger departures).
	boost::for_each(newDepartures | boost::adaptors::map_values, SortObjectList());

	// add data to departures
	return {
		newDepartures,
		FrameView(std::move(forwardsGlitz), std::move(reverseGlitz), std::move(guyInfo)),
		guyDepartureFrames,
		triggerSystemDepartureInformation.speedOfTime,
		triggerSystemDepartureInformation.paradoxPressure,
		winFrame};
}
} //namespace hg
