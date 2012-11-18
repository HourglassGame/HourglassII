#if 0
#include "BasicConfiguredTriggerSystem.h"
#include "ProtoStuff.h"
#include "Universe.h"
#include "CommonTriggerCode.h"

#include "Foreach.h"

namespace hg {
namespace {
struct GetTriggerValue : 
    std::unary_function<
        std::pair<int, std::vector<int> > const&,
        mt::std::vector<int>::type
    >
{
    mt::std::vector<int>::type operator()(
        std::pair<int, std::vector<int> > const& offsetAndTrigger) const
    {
        return mt::std::vector<int>::type(offsetAndTrigger.second.begin(), offsetAndTrigger.second.end());
    }
};

mt::std::vector<Collision>::type calculatePlatforms(
    std::vector<ProtoPlatform> const& protoPlatforms,
    mt::std::vector<mt::std::vector<int>::type>::type const& triggers)
{
    mt::std::vector<Collision>::type retv;
    retv.reserve(boost::distance(protoPlatforms));
    foreach (ProtoPlatform const& proto, protoPlatforms) {
        retv.push_back(proto.calculateCollision(triggers));
    }
    return retv;
}
mt::std::vector<PortalArea>::type calculatePortals(
    std::vector<ProtoPortal> const& protoPortals,
    mt::std::vector<Collision>::type const& collisionAreas)
{
    mt::std::vector<PortalArea>::type retv;
    retv.reserve(boost::distance(collisionAreas));
    foreach (ProtoPortal const& protoPortal, protoPortals) {
        retv.push_back(protoPortal.calculatePortalArea(collisionAreas));
    }
    return retv;
}

ArrivalLocation calculateArrivalLocation(PortalArea const& portal) {
    return ArrivalLocation(
        portal.getX(), portal.getY(),
        portal.getXspeed(), portal.getYspeed(),
        portal.getTimeDirection());
}

mt::std::vector<ArrivalLocation>::type calculateArrivalLocations(
    mt::std::vector<PortalArea>::type const& portals)
{
    mt::std::vector<ArrivalLocation>::type retv;
    retv.reserve(boost::distance(portals));
    foreach (PortalArea const& portal, portals) {
        retv.push_back(calculateArrivalLocation(portal));
    }
    return retv;
}

mt::std::vector<PositionAndVelocity2D>::type calculateButtons(
    std::vector<ProtoButton> const& protoButtons,
    mt::std::vector<Collision>::type const& collisions)
{
    mt::std::vector<PositionAndVelocity2D>::type retv;
    retv.reserve(boost::distance(collisions));
    foreach (ProtoButton const& protoButton, protoButtons) {
        retv.push_back(protoButton.calculatePositionAndVelocity2D(collisions));
    }
    return retv;
}
void fillPlatformTriggers(
    mt::std::vector<mt::std::vector<int>::type>::type& triggers,
    std::vector<ProtoPlatform> const& protoPlatforms,
    mt::std::vector<Collision>::type const& collisions)
{
    //assert(boost::distance(protoPlatforms) == boost::distance(collisions));
    for (std::size_t i(0), end(boost::distance(protoPlatforms)); i != end; ++i)
    {
        ProtoPlatform const& protoPlatform(protoPlatforms[i]);
        Collision const& collision(collisions[i]);
        mt::std::vector<int>::type& triggerData(triggers[protoPlatform.lastStateTriggerID_]);
        triggerData.reserve(4);
        triggerData.push_back(collision.getX());
        triggerData.push_back(collision.getY());
        triggerData.push_back(collision.getXspeed());
        triggerData.push_back(collision.getYspeed());
    }
}

mt::std::vector<RetardedNotActuallyAGlitzGlitz>::type
    calculatePlatformGlitz(mt::std::vector<Collision>::type const& collisions)
{
    mt::std::vector<RetardedNotActuallyAGlitzGlitz>::type retv;
    retv.reserve(boost::distance(collisions));
    foreach (Collision const& collision, collisions) {
        retv.push_back(
            RetardedNotActuallyAGlitzGlitz(
                collision.getX(), collision.getY(),
                collision.getWidth(), collision.getHeight(),
                collision.getXspeed(), collision.getYspeed(),
                0x32000000u, 0x00003200u,
                collision.getTimeDirection()));
    }
    return retv;
}

mt::std::vector<RetardedNotActuallyAGlitzGlitz>::type calculatePortalGlitz(mt::std::vector<PortalArea>::type const& portals)
{
    mt::std::vector<RetardedNotActuallyAGlitzGlitz>::type retv;
    retv.reserve(static_cast<unsigned long>(boost::distance(portals)));
    foreach (PortalArea const& portal, portals) {
        retv.push_back(
            RetardedNotActuallyAGlitzGlitz(
                portal.getX(), portal.getY(),
                portal.getWidth(), portal.getHeight(),
                portal.getXspeed(), portal.getYspeed(),
                0x78787800u, 0x78787800u,
                portal.getTimeDirection()));
    }
    return retv;
}

}//namespace

PhysicsAffectingStuff
    BasicConfiguredTriggerFrameState::calculatePhysicsAffectingStuff(
        Frame const* /*currentFrame*/,
        boost::transformed_range<
            GetBase<TriggerDataConstPtr>,
            mt::boost::container::vector<TriggerDataConstPtr>::type const > const& triggerArrivals/*,
            OperationInterrupter& interrupter*/)
{
    mt::std::vector<mt::std::vector<int>::type>::type apparentTriggers(
        calculateApparentTriggers(triggerSystem.triggerOffsetsAndDefaults, triggerArrivals));
    PhysicsAffectingStuff retv;
    //platforms always exist
    retv.collisions = calculatePlatforms(triggerSystem.protoPlatforms_, apparentTriggers);
    //portals always exist, because they can always be attached to platforms, and platforms always exit
    retv.portals = calculatePortals(triggerSystem.protoPortals_, retv.collisions);
    //Atm there is a 1-1 relationship between portals and arrival locations
    retv.arrivalLocations = calculateArrivalLocations(retv.portals);
    //store button positions for later...
    buttonStore = calculateButtons(triggerSystem.protoButtons_, retv.collisions);
    
    fillPlatformTriggers(triggers, triggerSystem.protoPlatforms_, retv.collisions);
    
    boost::push_back(glitzStore, calculatePlatformGlitz(retv.collisions));
    boost::push_back(glitzStore, calculatePortalGlitz(retv.portals));
    
    return retv;
}
namespace {
template<typename ObjectTB>
bool temporalIntersectingExclusive(ProtoButton const& protoA, PositionAndVelocity2D const& a, ObjectTB const& b)
{
    
    int xa;
    int ya;
    if (protoA.timeDirection_ == FORWARDS) {
        xa = a.getX();
        ya = a.getY();
    }
    else {
        xa = a.getX() - a.getXspeed();
        ya = a.getY() - a.getYspeed();
    }
    int wa(protoA.width_);
    int ha(protoA.height_);
    int xb;
    int yb;
    if (b.getTimeDirection() == FORWARDS) {
        xb = b.getX();
        yb = b.getY();
    }
    else {
        xb = b.getX() - b.getXspeed();
        yb = b.getY() - b.getYspeed();
    }
    int wb(b.getWidth());
    int hb(b.getHeight());
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


mt::std::vector<char>::type calculateButtonStates(
    std::vector<ProtoButton> const& protoButtons,
    mt::std::vector<PositionAndVelocity2D>::type const& buttonPositions,
    mt::boost::container::map<Frame*, ObjectList<Normal> >::type const& departures)
{
    //assert(boost::distance(protoButtons) == boost::distance(buttonPositions));
    mt::std::vector<char>::type buttonStates(boost::distance(protoButtons), false);
    for (std::size_t i(0), end(boost::distance(protoButtons)); i != end; ++i) {
        foreach (ObjectList<Normal> objectList, departures | boost::adaptors::map_values) {
        	foreach (Box const& box, objectList.getList<Box>()) {
                if (temporalIntersectingExclusive(protoButtons[i], buttonPositions[i], box)) {
                    goto intersecting;
                }
            }
        	foreach (Guy const& guy, objectList.getList<Guy>()) {
                if (temporalIntersectingExclusive(protoButtons[i], buttonPositions[i], guy)) {
                    goto intersecting;
                }
            }
        }
        continue;
        intersecting:
        buttonStates[i] = true;
    }
    return buttonStates;
}

mt::std::vector<RetardedNotActuallyAGlitzGlitz>::type calculateButtonGlitz(
    std::vector<ProtoButton> const& protoButtons,
    mt::std::vector<char>::type const& buttonStates,
    mt::std::vector<PositionAndVelocity2D>::type const& buttonStore)
{
    assert(boost::distance(protoButtons) == boost::distance(buttonStates));
    assert(boost::distance(protoButtons) == boost::distance(buttonStore));
    mt::std::vector<RetardedNotActuallyAGlitzGlitz>::type retv;
    retv.reserve(static_cast<unsigned long>(boost::distance(protoButtons)));
    for (std::size_t i(0), end(boost::distance(protoButtons)); i != end; ++i) {
        unsigned colour(buttonStates[i]?0x96FF9600u:0xFF969600u);
        retv.push_back(
            RetardedNotActuallyAGlitzGlitz(
                buttonStore[i].getX(), buttonStore[i].getY(),
                protoButtons[i].width_, protoButtons[i].height_,
                buttonStore[i].getXspeed(), buttonStore[i].getYspeed(),
                colour, colour,
                protoButtons[i].timeDirection_));
    }
    return retv;
}

void fillButtonTriggers(
    mt::std::vector<mt::std::vector<int>::type>::type& triggers,
    std::vector<ProtoButton> const& protoButtons,
    mt::std::vector<char>::type const& states)
{
    for (std::size_t i(0), end(boost::distance(protoButtons)); i != end; ++i)
    {
        triggers[protoButtons[i].triggerID_].push_back(states[i]);
    }
}
mt::std::vector<TriggerData>::type
    createTriggerListFromTriggerValueList(
        mt::std::vector<mt::std::vector<int>::type>::type const& triggerValues)
{
    mt::std::vector<TriggerData>::type retv;
    retv.reserve(triggerValues.size());
    for(std::size_t i(0), end(triggerValues.size()); i != end; ++i)
    {
        retv.push_back(TriggerData(i, triggerValues[i]));
    }
    return retv;
}

}//namespace


TriggerFrameStateImplementation::DepartureInformation
    BasicConfiguredTriggerFrameState::getDepartureInformation(
        mt::boost::container::map<Frame*, ObjectList<Normal> >::type const& departures,
        Frame* currentFrame/*,
        OperationInterrupter& interrupter*/)
{
    //These are the states that are apparent in the buttonGlitz
    //These are also the states that travel through time to affect
    //platforms at their nextframes.
    mt::std::vector<char>::type buttonStates(calculateButtonStates(triggerSystem.protoButtons_, buttonStore, departures));

    boost::push_back(glitzStore, calculateButtonGlitz(triggerSystem.protoButtons_, buttonStates, buttonStore));
    
    fillButtonTriggers(triggers, triggerSystem.protoButtons_, buttonStates);
    
    return
        DepartureInformation(
            calculateActualTriggerDepartures(
                createTriggerListFromTriggerValueList(triggers), triggerSystem.triggerOffsetsAndDefaults, currentFrame),
                glitzStore,
                mt::std::vector<RetardedNotActuallyAGlitzGlitz>::type(),//foregroundGlitz
                mt::std::vector<ObjectAndTime<Box, Frame*> >::type());
}

BasicConfiguredTriggerFrameState::BasicConfiguredTriggerFrameState(BasicConfiguredTriggerSystem const& triggerSys) :
    buttonStore(),
    triggers(boost::distance(triggerSys.triggerOffsetsAndDefaults)),
    glitzStore(),
    triggerSystem(triggerSys)
{
}
}//namespace hg
#endif
