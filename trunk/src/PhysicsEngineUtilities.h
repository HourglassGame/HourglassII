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

#include "PhysicsObjectType.h"

#include <vector>
#include "mt/boost/container/map.hpp"
#include "mt/std/vector"
#include <utility>

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
    Environment const &env,
    RandomAccessGuyRange const &guyArrivalList,
    Frame *frame,
    std::vector<InputList> const &playerInput,
    mt::std::vector<ObjectAndTime<Guy, Frame*>> &nextGuy,
    mt::std::vector<ObjectAndTime<Box, Frame*>> &nextBox,
    mt::std::vector<char> &nextBoxNormalDeparture,
    mt::std::vector<Collision> const &nextPlatform,
    mt::std::vector<PortalArea> const &nextPortal,
    mt::std::vector<ArrivalLocation> const &arrivalLocations,
    mt::std::vector<MutatorArea> const &mutators,
    TriggerFrameState &triggerFrameState,
    GuyGlitzAdder const &guyGlitzAdder,
    bool &nextPlayerFrame,
    bool &winFrame);

template <
    typename RandomAccessBoxRange,
    typename RandomAccessPortalRange,
    typename RandomAccessPlatformRange,
    typename RandomAccessArrivalLocationRange,
    typename RandomAccessMutatorRange,
    typename FrameT>
void boxCollisionAlogorithm(
    Environment const &env,
    RandomAccessBoxRange const &boxArrivalList,
    mt::std::vector<Box> const &additionalBox,
    mt::std::vector<ObjectAndTime<Box, FrameT> > &nextBox,
    mt::std::vector<char> &nextBoxNormalDeparture,
    RandomAccessPlatformRange const &nextPlatform,
    RandomAccessPortalRange const &nextPortal,
    RandomAccessArrivalLocationRange const &arrivalLocations,
    RandomAccessMutatorRange const &mutators,
    TriggerFrameState &triggerFrameState,
    FrameT const &frame);
    
void makeBoxGlitzListForNormalDepartures(
    mt::std::vector<ObjectAndTime<Box, Frame*> > const &nextBox,
    mt::std::vector<char> &nextBoxNormalDeparture,
    BoxGlitzAdder const &boxGlitzAdder);
    
template <
    typename RandomAccessPortalRange,
    typename RandomAccessMutatorRange,
    typename FrameT>
void makeBoxAndTimeWithPortalsAndMutators(
    mt::std::vector<ObjectAndTime<Box, FrameT> > &nextBox,
    mt::std::vector<char> &nextBoxNormalDeparture,
    RandomAccessPortalRange const &portals,
    RandomAccessMutatorRange const &mutators,
    int x,
    int y,
    int xspeed,
    int yspeed,
    int size,
    int oldIllegalPortal,
    TimeDirection oldTimeDirection,
    TriggerFrameState &triggerFrameState,
    FrameT frame);

bool explodeBoxesUpwards(
    mt::std::vector<int> &x,
    mt::std::vector<int> const &xTemp,
    mt::std::vector<int> &y,
    mt::std::vector<int> const &size,
    mt::std::vector<mt::std::vector<std::size_t>> const &links,
    bool firstTime,
    mt::std::vector<char> &toBeSquished,
    mt::std::vector<std::pair<bool, int> > const &bound,
    std::size_t index,
    int boundSoFar);


bool explodeBoxes(
    mt::std::vector<int> &pos,
    mt::std::vector<int> const &size,
    mt::std::vector<mt::std::vector<std::size_t>> const &links,
    mt::std::vector<char> &toBeSquished,
    mt::std::vector<std::pair<bool, int> > const &bound,
    std::size_t index,
    int boundSoFar,
    int sign);

void recursiveBoxCollision(
    mt::std::vector<int> &majorAxis,
    mt::std::vector<int> const &minorAxis,
    mt::std::vector<int> const &size,
    mt::std::vector<char> const &squished,
    mt::std::vector<std::size_t> &boxesSoFar,
    std::size_t index,
    int subtractionNumber);

template<typename RandomAccessObjectAndTypeRange>
void buildDeparturesForComplexEntities(
    RandomAccessObjectAndTypeRange const &next,
    PhysicsEngine::FrameDepartureT &newDepartures);

void buildDepartures(
    mt::std::vector<ObjectAndTime<Box, Frame*> > const &nextBox,
    mt::std::vector<ObjectAndTime<Guy, Frame*> > const &nextGuy,
    PhysicsEngine::FrameDepartureT &newDepartures,
    Frame *frame);

//bool wallAtInclusive(Environment const &env, int x, int y, int w, int h);
bool wallAtExclusive(Wall const &wall, int x, int y, int w, int h);
bool wallAtExclusive(Environment const &env, int x, int y, int w, int h);

bool getRaytraceRectangleCollision(
    int sx, int sy, 
    int &px, int &py,
    int left, int top,
    int width, int height,
    int dx, int dy,
    bool mostlySideways);

void doGunRaytrace(
    PhysicsObjectType &targetType,
    int &targetId,
    Environment const &env,
    int &sx, int &sy, int &px, int &py,
    mt::std::vector<Collision> const &nextPlatform,
    mt::std::vector<ObjectAndTime<Box, Frame*> > box,
    mt::std::vector<char> &nextBoxNormalDeparture,
    mt::std::vector<int> gx, // other guy things
    mt::std::vector<int> gy,
    mt::std::vector<int> gw,
    mt::std::vector<int> gh,
    mt::std::vector<char> shootable);
    
int RectangleIntersectionDirection(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
bool IsPointInVerticalQuadrant(int x, int y, int x1, int y1, int w, int h);
bool PointInRectangleInclusive(int px, int py, int x, int y, int w, int h);
bool PointInRectangleExclusive(int px, int py, int x, int y, int w, int h);
bool IntersectingRectanglesInclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
bool IntersectingRectanglesExclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
bool IntersectingRectanglesInclusiveCollisionOverlap(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2, int buffer);
bool RectangleWithinInclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);

template<typename RandomAccessGuyRange>
bool currentPlayerInArrivals(RandomAccessGuyRange const &guyArrivals, std::size_t playerInputSize);
enum {
    COLLISION_BUFFER_RANGE = 100,
    HALF_COLLISION_BUFFER_RANGE = 50,
    REVERSE_PLATFORM_CHRONOFRAG_FUDGE = 250
};
}
#include "PhysicsEngineUtilities_def.h"
#endif //HG_PHYSICS_ENGINE_TEST_H
