#ifndef HG_PHYSICS_ENGINE_TEST_H
#define HG_PHYSICS_ENGINE_TEST_H

#include "InputList.h"
#include "ObjectList.h"
#include "ObjectPtrList.h"
#include "ObjectListTypes.h"
#include "TriggerSystem.h"
#include "Environment.h"
#include "multi_thread_allocator.h"
#include "BoxGlitzAdder.h"
#include "GuyGlitzAdder.h"

#include <vector>
#include "mt/boost/container/map.hpp"
#include "mt/std/vector"
#include <utility>
#include <boost/move/move.hpp>

#include "Frame_fwd.h"

#include "Frame.h"
#include "Universe.h"
#include "Geometry.h"

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

#include "PhysicsEngine.h"

#include <iostream>

#include <cassert>
namespace hg {
struct SortObjectList {
    void operator()(ObjectList<Normal>& toSort) const {
        toSort.sort();
    }
};

template<typename RandomAccessGuyRange>
void guyStep(
    Environment const& env,
    RandomAccessGuyRange const& guyArrivalList,
    Frame* frame,
    std::vector<InputList> const& playerInput,
    mt::std::vector<ObjectAndTime<Guy, Frame*> >::type& nextGuy,
    mt::std::vector<ObjectAndTime<Box, Frame*> >::type& nextBox,
    mt::std::vector<char>::type& nextBoxNormalDeparture,
    mt::std::vector<Collision>::type const& nextPlatform,
    mt::std::vector<PortalArea>::type const& nextPortal,
    mt::std::vector<ArrivalLocation>::type const& arrivalLocations,
    mt::std::vector<MutatorArea>::type const& mutators,
    TriggerFrameState& triggerFrameState,
    GuyGlitzAdder const& guyGlitzAdder,
    BoxGlitzAdder const& boxGlitzAdder,
    bool& nextPlayerFrame,
    bool& winFrame);

template <
    typename RandomAccessBoxRange,
    typename RandomAccessPortalRange,
    typename RandomAccessPlatformRange,
    typename RandomAccessArrivalLocationRange,
    typename RandomAccessMutatorRange,
    typename FrameT>
void boxCollisionAlogorithm(
    Environment const& env,
    RandomAccessBoxRange const& boxArrivalList,
    mt::std::vector<Box>::type const& additionalBox,
    typename mt::std::vector<ObjectAndTime<Box, FrameT> >::type& nextBox,
    mt::std::vector<char>::type& nextBoxNormalDeparture,
    BoxGlitzAdder const& boxGlitzAdder,
    RandomAccessPlatformRange const& nextPlatform,
    RandomAccessPortalRange const& nextPortal,
    RandomAccessArrivalLocationRange const& arrivalLocations,
    RandomAccessMutatorRange const& mutators,
    TriggerFrameState& triggerFrameState,
    FrameT const& frame);

template <
    typename RandomAccessPortalRange,
    typename RandomAccessMutatorRange,
    typename FrameT>
void makeBoxAndTimeWithPortalsAndMutators(
    typename mt::std::vector<ObjectAndTime<Box, FrameT> >::type& nextBox,
    mt::std::vector<char>::type& nextBoxNormalDeparture,
    const RandomAccessPortalRange& portals,
    const RandomAccessMutatorRange& mutators,
    int x,
    int y,
    int xspeed,
    int yspeed,
    int size,
    int oldIllegalPortal,
    TimeDirection oldTimeDirection,
    TriggerFrameState& triggerFrameState,
    BoxGlitzAdder const& boxGlitzAdder,
    FrameT frame);

bool explodeBoxesUpwards(
    mt::std::vector<int>::type& x,
    mt::std::vector<int>::type const& xTemp,
    mt::std::vector<int>::type& y,
    mt::std::vector<int>::type const& size,
    mt::std::vector<mt::std::vector<std::size_t>::type >::type const& links,
    bool firstTime,
    mt::std::vector<char>::type& toBeSquished,
    mt::std::vector<std::pair<bool, int> >::type const& bound,
    std::size_t index,
    int boundSoFar);


bool explodeBoxes(
    mt::std::vector<int>::type& pos,
    mt::std::vector<int>::type const& size,
    mt::std::vector<mt::std::vector<std::size_t>::type >::type const& links,
    mt::std::vector<char>::type& toBeSquished,
    mt::std::vector<std::pair<bool, int> >::type const& bound,
    std::size_t index,
    int boundSoFar,
    int sign);

void recursiveBoxCollision(
    mt::std::vector<int>::type& majorAxis,
    mt::std::vector<int>::type const& minorAxis,
    mt::std::vector<int>::type const& size,
    mt::std::vector<char>::type const& squished,
    mt::std::vector<std::size_t>::type& boxesSoFar,
    std::size_t index,
    int subtractionNumber);
    
template<typename RandomAccessObjectAndTypeRange>
void buildDeparturesForComplexEntities(
    RandomAccessObjectAndTypeRange const& next,
    PhysicsEngine::FrameDepartureT& newDepartures);
    
void buildDepartures(
    mt::std::vector<ObjectAndTime<Box, Frame*> >::type const& nextBox,
    mt::std::vector<ObjectAndTime<Guy, Frame*> >::type const& nextGuy,
    PhysicsEngine::FrameDepartureT& newDepartures,
    Frame* frame);
    
//bool wallAtInclusive(const Environment& env, int x, int y, int w, int h);
bool wallAtExclusive(Wall const& wall, int x, int y, int w, int h);
bool wallAtExclusive(const Environment& env, int x, int y, int w, int h);
    
unsigned int Raytrace(
        Environment const& env,
        int sx, int sy, int px, int py,
        mt::std::vector<ObjectAndTime<Box, Frame*> >::type box,
        mt::std::vector<char>::type& nextBoxNormalDeparture,
        mt::std::vector<int>::type gx, // other guy things
        mt::std::vector<int>::type gy,
        mt::std::vector<int>::type gw,
        mt::std::vector<int>::type gh,
        mt::std::vector<char>::type shootable,
        int myIndex);

bool IsPointInVerticalQuadrant(int x, int y, int x1, int y1, int w, int h);
bool PointInRectangleInclusive(int px, int py, int x, int y, int w, int h);
bool IntersectingRectanglesInclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
bool IntersectingRectanglesExclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
bool IntersectingRectanglesInclusiveCollisionOverlap(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2, int buffer);
bool RectangleWithinInclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
FrameView makeFrameView(
    mt::std::vector<GuyOutputInfo>::type const& guyInfo,
    mt::std::vector<RectangleGlitz>::type const& backgroundTriggerGlitz,
    mt::std::vector<RectangleGlitz>::type const& foregroundTriggerGlitz,
    mt::std::vector<Glitz>::type const& forwardsGlitz,
    mt::std::vector<Glitz>::type const& reverseGlitz);
template<typename RandomAccessGuyRange>
bool currentPlayerInArrivals(RandomAccessGuyRange const& guyArrivals, std::size_t playerInputSize);
enum {
	JUMP_SPEED 	= -550,
	COLLISION_BUFFER_RANGE = 100,
	HALF_COLLISION_BUFFER_RANGE = 50
};
}
#include "PhysicsEngineUtilities_def.h"
#endif //HG_PHYSICS_ENGINE_TEST_H
