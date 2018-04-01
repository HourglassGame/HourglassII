#ifndef HG_PHYSICS_AFFECTING_STUFF_H
#define HG_PHYSICS_AFFECTING_STUFF_H
#include "mt/std/vector"
#include "mp/std/vector"
#include "Box.h"
#include "Collision.h"
#include "PortalArea.h"
#include "MutatorArea.h"
#include "ArrivalLocation.h"
namespace hg {
struct PhysicsAffectingStuff final {
    PhysicsAffectingStuff(memory_pool<user_allocator_tbb_alloc> &pool) :
        additionalBoxes(pool),
        portals(pool),
        collisions(pool),
        mutators(pool),
        arrivalLocations(pool)
    {}

    mp::std::vector<Box> additionalBoxes;
    mp::std::vector<PortalArea> portals;
    mp::std::vector<Collision> collisions;
    mp::std::vector<MutatorArea> mutators;
    //guaranteed to always contain elements at each index that could possibly be indexed
    //ie- arrivalLocations will always be the same length for a particular TriggerSystem
    mp::std::vector<ArrivalLocation> arrivalLocations;
};
}
#endif //HG_PHYSICS_AFFECTING_STUFF_H
