#ifndef HG_PHYSICS_AFFECTING_STUFF_H
#define HG_PHYSICS_AFFECTING_STUFF_H
#include "mt/std/vector"
#include "Box.h"
#include "Collision.h"
#include "PortalArea.h"
#include "MutatorArea.h"
#include "ArrivalLocation.h"
namespace hg {
struct PhysicsAffectingStuff final {
    mt::std::vector<Box> additionalBoxes;
    mt::std::vector<PortalArea> portals;
    mt::std::vector<Collision> collisions;
    mt::std::vector<MutatorArea> mutators;
    //guaranteed to always contain elements at each index that could possibly be indexed
    //ie- arrivalLocations will always be the same length for a particular TriggerSystem
    mt::std::vector<ArrivalLocation> arrivalLocations;
};
}
#endif //HG_PHYSICS_AFFECTING_STUFF_H
