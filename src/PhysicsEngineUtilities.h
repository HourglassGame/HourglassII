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
#include "mp/std/vector"
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
struct SortObjectList final {
    void operator()(ObjectList<Normal>& toSort) const {
        toSort.sort();
    }
};

template<
    typename RandomAccessGuyRange,
    typename RandomAccessBoxRange>
void guyStep(
    Environment const &env,
    RandomAccessGuyRange const &guyArrivalList,
    Frame *frame,
    std::vector<GuyInput> const &playerInput,
    mp::std::vector<ObjectAndTime<Guy, Frame*>> &nextGuy,
    mp::std::vector<ObjectAndTime<Box, Frame*>> &nextBox,
    mp::std::vector<char> &nextBoxNormalDeparture,
    RandomAccessBoxRange const &boxArrivalList,
    mp::std::vector<Collision> const &nextPlatform,
    mp::std::vector<PortalArea> const &nextPortal,
    mp::std::vector<ArrivalLocation> const &arrivalLocations,
    mp::std::vector<MutatorArea> const &mutators,
    TriggerFrameState &triggerFrameState,
    GuyGlitzAdder const &guyGlitzAdder,
    bool &nextPlayerFrame,
    bool &winFrame,
    memory_pool<user_allocator_tbb_alloc> &pool);

template <
    typename RandomAccessPlatformRange>
void boxInteractionBoundLoop(
    TimeDirection const boxDirection,
    Environment const &env,
    mp::std::vector<int> &x,
    mp::std::vector<int> &y,
    mp::std::vector<int> &xTemp,
    mp::std::vector<int> &yTemp,
    mp::std::vector<char> &squished,
    mp::std::vector<int> const &size,
    mp::std::vector<Box> const &oldBoxList,
    RandomAccessPlatformRange const &nextPlatform,
    BoxGlitzAdder const &boxGlitzAdder,
    memory_pool<user_allocator_tbb_alloc> &pool);

template <
    typename RandomAccessBoxRange,
    typename RandomAccessPortalRange,
    typename RandomAccessPlatformRange,
    typename RandomAccessArrivalLocationRange,
    typename RandomAccessMutatorRange,
    typename FrameT>
void boxCollisionAlgorithm(
    Environment const &env,
    RandomAccessBoxRange const &boxArrivalList,
    mp::std::vector<Box> const &additionalBox,
    mp::std::vector<ObjectAndTime<Box, FrameT> > &nextBox,
    mp::std::vector<char> &nextBoxNormalDeparture,
    RandomAccessPlatformRange const &nextPlatform,
    RandomAccessPortalRange const &nextPortal,
    RandomAccessArrivalLocationRange const &arrivalLocations,
    RandomAccessMutatorRange const &mutators,
    TriggerFrameState &triggerFrameState,
    BoxGlitzAdder const &boxGlitzAdder,
    FrameT const &frame,
    memory_pool<user_allocator_tbb_alloc> &pool);
    
void makeBoxGlitzListForNormalDepartures(
    mp::std::vector<ObjectAndTime<Box, Frame*>> const &nextBox,
    mp::std::vector<char> &nextBoxNormalDeparture,
    BoxGlitzAdder const &boxGlitzAdder);
    
template <
    typename RandomAccessPortalRange,
    typename RandomAccessMutatorRange,
    typename FrameT>
void makeBoxAndTimeWithPortalsAndMutators(
    mp::std::vector<ObjectAndTime<Box, FrameT>> &nextBox,
    mp::std::vector<char> &nextBoxNormalDeparture,
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
    FrameT frame,
    memory_pool<user_allocator_tbb_alloc> &pool);

bool explodeBoxesUpwards(
    mp::std::vector<int> &x,
    mp::std::vector<int> const &xTemp,
    mp::std::vector<int> &y,
    mp::std::vector<int> const &size,
    mp::std::vector<mp::std::vector<std::size_t>> const &links,
    bool firstTime,
    mp::std::vector<char> &toBeSquished,
    mp::std::vector<std::pair<bool, int> > const &bound,
    std::size_t index,
    int boundSoFar);


bool explodeBoxes(
    mp::std::vector<int> &pos,
    mp::std::vector<int> const &size,
    mp::std::vector<mp::std::vector<std::size_t>> const &links,
    mp::std::vector<char> &toBeSquished,
    mp::std::vector<std::pair<bool, int>> const &bound,
    std::size_t index,
    int boundSoFar,
    int sign);

void recursiveBoxCollision(
    mp::std::vector<int> &majorAxis,
    mp::std::vector<int> const &minorAxis,
    mp::std::vector<int> const &size,
    mp::std::vector<char> const &squished,
    mp::std::vector<std::size_t> &boxesSoFar,
    std::size_t index,
    int subtractionNumber,
    TimeDirection const boxDirection,
    mp::std::vector<Box> const &oldBoxList);

template<typename RandomAccessObjectAndTypeRange>
void buildDeparturesForComplexEntities(
    RandomAccessObjectAndTypeRange const &next,
    PhysicsEngine::FrameDepartureT &newDepartures);

template<typename RandomAccessObjectAndTypeRange>
void buildDeparturesForComplexEntitiesWithIndexCaching(
    RandomAccessObjectAndTypeRange const &next,
    mt::std::vector<std::tuple<std::size_t, Frame *>> &departureFrames,
    PhysicsEngine::FrameDepartureT &newDepartures);

void buildDepartures(
    mp::std::vector<ObjectAndTime<Box, Frame*> > const &nextBox,
    mp::std::vector<ObjectAndTime<Guy, Frame*> > const &nextGuy,
    mt::std::vector<std::tuple<std::size_t, Frame *>> &guyDepartureFrames,
    PhysicsEngine::FrameDepartureT &newDepartures,
    Frame *frame);

//bool wallAtInclusive(Environment const &env, int x, int y, int w, int h);
bool wallAtExclusive(Wall const &wall, int x, int y, int w, int h);
bool wallAtExclusive(Environment const &env, int x, int y, int w, int h);

struct GunRaytraceResult final {
    PhysicsObjectType targetType;
    std::size_t targetId;
    int px;
    int py;
};

GunRaytraceResult doGunRaytrace(
    int const sx, int const sy,
    int const aimx_raw, int const aimy,

    //Env data
    Wall const &wall,

    //Platform data
    mp::std::vector<Collision> const &nextPlatform,

    //Box Data
    mp::std::vector<ObjectAndTime<Box, Frame *>> const &nextBox,
    mp::std::vector<char> const &nextBoxNormalDeparture,

    //Guy Data
    mp::std::vector<int> const &gx,
    mp::std::vector<int> const &gy,
    mp::std::vector<int> const &gw,
    mp::std::vector<int> const &gh,
    mp::std::vector<char> const &shootable,

    memory_pool<user_allocator_tbb_alloc> &pool
);
    
int RectangleIntersectionDirection(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
bool IsPointInVerticalQuadrant(int x, int y, int x1, int y1, int w, int h);
bool PointInRectangleInclusive(int px, int py, int x, int y, int w, int h);
bool PointInRectangleSemiInclusive(int px, int py, int x, int y, int w, int h);
bool PointInRectangleExclusive(int px, int py, int x, int y, int w, int h);
bool IntersectingRectanglesInclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
bool IntersectingRectanglesExclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
bool IntersectingRectanglesInclusiveCollisionOverlap(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2, int buffer);
bool RectangleWithinInclusive(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);

struct RaytraceRectangleCollisionResult final {
    //TODO: consider making this a variant/optional; since `point` should not be accessed unless `hit` is true
    bool const hit;
    vector2<int> const point;
};
inline bool operator==(RaytraceRectangleCollisionResult const& l, RaytraceRectangleCollisionResult const& r) {
    return l.hit == r.hit && (l.hit == false || l.point.x == r.point.x && l.point.y == r.point.y);
}

RaytraceRectangleCollisionResult getRaytraceRectangleCollision(
    int const sx, int const sy,
    int const aimx, int const aimy,
    int const left, int const top,
    int const width, int const height
);

template<typename RandomAccessGuyRange>
bool currentPlayerInArrivals(RandomAccessGuyRange const &guyArrivals, std::size_t playerInputSize);
enum {
    COLLISION_BUFFER_RANGE = 100,
    HALF_COLLISION_BUFFER_RANGE = 50
};

vector2<int> doGunWallRaytrace(
    Wall const &wall,
    int const sx, int const sy,
    int const aimx, int const aimy,
    memory_pool<user_allocator_tbb_alloc> &pool);
}
#include "PhysicsEngineUtilities_def.h"
#endif //HG_PHYSICS_ENGINE_TEST_H
