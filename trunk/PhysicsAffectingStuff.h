#ifndef HG_PHYSICS_AFFECTING_STUFF_H
#define HG_PHYSICS_AFFECTING_STUFF_H
#include "mt/std/vector"
#include "Box.h"
#include "Collision.h"
#include "PortalArea.h"
#include "MutatorArea.h"
#include "ArrivalLocation.h"
namespace hg {
struct PhysicsAffectingStuff {
    mt::std::vector<Box>::type additionalBoxes;
    mt::std::vector<PortalArea>::type portals;
    mt::std::vector<Collision>::type collisions;
    mt::std::vector<MutatorArea>::type mutators;
    //guaranteed to always contain elements at each index that could possibly be indexed
    //ie- arrivalLocations will always be the same length for a particular NewTriggerSystem
    mt::std::vector<ArrivalLocation>::type arrivalLocations;
};
}
#endif //HG_PHYSICS_AFFECTING_STUFF_H
