#include "TriggerSystem.h"
#include "Frame.h"
#include "Universe.h"
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/adaptor/indirected.hpp>
#include <boost/range/algorithm/find.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/call_traits.hpp>
#include <boost/foreach.hpp>
#include <boost/range.hpp>
#define foreach BOOST_FOREACH
namespace hg {
Collision ProtoPlatform::calculateCollision(
        mt::boost::container::vector<
            mt::boost::container::vector<int>::type
        >::type const& triggers) const
{
        PlatformDestination const& destination(
            triggers[buttonTriggerID_].front() ?
            destinations_.onDestination :
            destinations_.offDestination);
        
        mt::boost::container::vector<int>::type const& lastStateTrigger(triggers[lastStateTriggerID_]);
        
        PositionAndVelocity horizontal(
            destination.
                getXDestination().
                    solvePDEquation(
                        PositionAndVelocity(lastStateTrigger[0], lastStateTrigger[2])));

        PositionAndVelocity vertical(
            destination.
                getYDestination().
                    solvePDEquation(
                        PositionAndVelocity(lastStateTrigger[1], lastStateTrigger[3])));
        
        return
            Collision(
                horizontal.position, vertical.position,
                horizontal.velocity, vertical.velocity,
                width_, height_,
                timeDirection_);
}

PortalArea ProtoPortal::calculatePortalArea(
        mt::boost::container::vector<Collision>::type const& collisions) const
{
    int x;
    int y;
    int xSpeed;
    int ySpeed;
    if (attachment_.platformIndex != std::numeric_limits<std::size_t>::max()) {
        Collision const& collision(collisions[attachment_.platformIndex]);     
        if (collision.getTimeDirection() * timeDirection_ == FORWARDS) {
            x = collision.getX() + attachment_.xOffset;
            y = collision.getY() + attachment_.yOffset;
            xSpeed = collision.getXspeed();
            ySpeed = collision.getYspeed();
        }
        else {
            x = collision.getX() - collision.getXspeed() + attachment_.xOffset;
            y = collision.getY() - collision.getXspeed() + attachment_.yOffset;
            //Speed should be departure location - arrival location, but unfortunately
            //portals neither depart nor arrive. speed = -collision.getspeed(); gives the correct result
            //See this example:
            /*
            frame 0: plat arrives at 2 goes to 5 plat speed = 3
            frame 1: plat arrives at 5 goes to 6 plat speed = 1
            frame 2: plat arrives at 6 goes to 14 plat speed = 8
            
            
            frame 2: portal arrives at ? goes to 6 portal speed = ?
            frame 1: portal arrives at 6 goes to 5 portal speed = -1 //(== -plat speed)
            frame 0: portal arrives at 5 goes to 2 portal speed = -3 //(== -plat speed)
            */
            xSpeed = -collision.getXspeed();
            ySpeed = -collision.getYspeed();
        }

    }
    else {
        x = attachment_.xOffset;
        y = attachment_.yOffset;
        xSpeed = 0;
        ySpeed = 0;
    }
    return PortalArea(
        x,
        y,
        width_,
        height_,
        xSpeed,
        ySpeed,
        timeDirection_,
        destinationIndex_,
        xDestination_,
        yDestination_,
        relativeTime_,
        timeDestination_,
        illegalDestination_,
        fallable_,
        winner_);
}

PositionAndVelocity2D ProtoButton::calculatePositionAndVelocity2D(
        mt::boost::container::vector<Collision>::type const& collisions) const
{
    int x;
    int y;
    int xSpeed;
    int ySpeed;
    if (attachment_.platformIndex != std::numeric_limits<std::size_t>::max()) {
        Collision const& collision(collisions[attachment_.platformIndex]);
        if (collision.getTimeDirection() * timeDirection_ == FORWARDS) {
            x = collision.getX() + attachment_.xOffset;
            y = collision.getY() + attachment_.yOffset;
            xSpeed = collision.getXspeed();
            ySpeed = collision.getYspeed();
        }
        else {
            x = collision.getX() - collision.getXspeed() + attachment_.xOffset;
            y = collision.getY() - collision.getYspeed() + attachment_.yOffset;
            xSpeed = -collision.getXspeed();
            ySpeed = -collision.getYspeed();
        }
    }
    else {
        x = attachment_.xOffset;
        y = attachment_.yOffset;
        xSpeed = 0;
        ySpeed = 0;
    }
    return PositionAndVelocity2D(
        x,
        y,
        xSpeed,
        ySpeed);
}


namespace {
struct GetTriggerValue : 
    std::unary_function<
        std::pair<int, boost::container::vector<int> > const&,
        mt::boost::container::vector<int>::type
    >
{
    mt::boost::container::vector<int>::type operator()(
        std::pair<int, boost::container::vector<int> > const& offsetAndTrigger) const
    {
        return mt::boost::container::vector<int>::type(offsetAndTrigger.second.begin(), offsetAndTrigger.second.end());
    }
};

mt::boost::container::vector<Collision>::type calculatePlatforms(
    boost::container::vector<ProtoPlatform> const& protoPlatforms,
    mt::boost::container::vector<mt::boost::container::vector<int>::type>::type const& triggers)
{
    mt::boost::container::vector<Collision>::type retv;
    retv.reserve(boost::distance(protoPlatforms));
    foreach (ProtoPlatform const& proto, protoPlatforms) {
        retv.push_back(proto.calculateCollision(triggers));
    }
    return retv;
}
mt::boost::container::vector<PortalArea>::type calculatePortals(
    boost::container::vector<ProtoPortal> const& protoPortals,
    mt::boost::container::vector<Collision>::type const& collisionAreas)
{
    mt::boost::container::vector<PortalArea>::type retv;
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

mt::boost::container::vector<ArrivalLocation>::type calculateArrivalLocations(
    mt::boost::container::vector<PortalArea>::type const& portals)
{
    mt::boost::container::vector<ArrivalLocation>::type retv;
    retv.reserve(boost::distance(portals));
    foreach (PortalArea const& portal, portals) {
        retv.push_back(calculateArrivalLocation(portal));
    }
    return retv;
}

mt::boost::container::vector<PositionAndVelocity2D>::type calculateButtons(
    boost::container::vector<ProtoButton> const& protoButtons,
    mt::boost::container::vector<Collision>::type const& collisions)
{
    mt::boost::container::vector<PositionAndVelocity2D>::type retv;
    retv.reserve(boost::distance(collisions));
    foreach (ProtoButton const& protoButton, protoButtons) {
        retv.push_back(protoButton.calculatePositionAndVelocity2D(collisions));
    }
    return retv;
}
void fillPlatformTriggers(
    mt::boost::container::vector<mt::boost::container::vector<int>::type>::type& triggers,
    boost::container::vector<ProtoPlatform> const& protoPlatforms,
    mt::boost::container::vector<Collision>::type const& collisions)
{
    //assert(boost::distance(protoPlatforms) == boost::distance(collisions));
    for (std::size_t i(0), end(boost::distance(protoPlatforms)); i != end; ++i)
    {
        ProtoPlatform const& protoPlatform(protoPlatforms[i]);
        Collision const& collision(collisions[i]);
        mt::boost::container::vector<int>::type& triggerData(triggers[protoPlatform.lastStateTriggerID_]);
        triggerData.reserve(4);
        triggerData.push_back(collision.getX());
        triggerData.push_back(collision.getY());
        triggerData.push_back(collision.getXspeed());
        triggerData.push_back(collision.getYspeed());
    }
}

mt::boost::container::vector<RectangleGlitz>::type
    calculatePlatformGlitz(mt::boost::container::vector<Collision>::type const& collisions)
{
    mt::boost::container::vector<RectangleGlitz>::type retv;
    retv.reserve(boost::distance(collisions));
    foreach(Collision const& collision, collisions) {
        retv.push_back(
            RectangleGlitz(
                collision.getX(), collision.getY(),
                collision.getWidth(), collision.getHeight(),
                collision.getXspeed(), collision.getYspeed(),
                0x32000000u, 0x00003200u,
                collision.getTimeDirection()));
    }
    return retv;
}

mt::boost::container::vector<RectangleGlitz>::type calculatePortalGlitz(mt::boost::container::vector<PortalArea>::type const& portals)
{
    mt::boost::container::vector<RectangleGlitz>::type retv;
    retv.reserve(boost::distance(portals));
    foreach(PortalArea const& portal, portals) {
        retv.push_back(
            RectangleGlitz(
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
    NewOldTriggerFrameState::calculatePhysicsAffectingStuff(
        boost::transformed_range<
            GetBase<TriggerDataConstPtr>,
            mt::boost::container::vector<TriggerDataConstPtr>::type const > const& triggerArrivals)
{
    typedef boost::transformed_range<
        GetBase<TriggerDataConstPtr>,
        boost::container::vector<TriggerDataConstPtr> const > TriggerDataConstPtrRange;
    //trigger arrivals with defaults for places where none arrived in triggerArrivals
    //index field replaced by position in list.
    mt::boost::container::vector<mt::boost::container::vector<int>::type>::type apparentTriggers;
    apparentTriggers.reserve(boost::distance(triggerSystem.triggerOffsetsAndDefaults));
    typedef std::pair<int, boost::container::vector<int> > TriggerOffsetAndDefault;
    foreach (TriggerOffsetAndDefault const& offsetAndDefault, triggerSystem.triggerOffsetsAndDefaults) {
        apparentTriggers.push_back(
            mt::boost::container::vector<int>::type(
                    offsetAndDefault.second.begin(),
                    offsetAndDefault.second.end()));
    }
    
    foreach (TriggerDataConstPtrRange::value_type const& arrival, triggerArrivals) {
        apparentTriggers[arrival.getIndex()] = arrival.getValue();
    }
    
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


mt::boost::container::vector<char>::type calculateButtonStates(
    boost::container::vector<ProtoButton> const& protoButtons,
    mt::boost::container::vector<PositionAndVelocity2D>::type const& buttonPositions,
    mt::boost::container::map<Frame*, ObjectList<Normal> >::type const& departures)
{
    //assert(boost::distance(protoButtons) == boost::distance(buttonPositions));
    mt::boost::container::vector<char>::type buttonStates(boost::distance(protoButtons), false);
    for (std::size_t i(0), end(boost::distance(protoButtons)); i != end; ++i) {
        foreach(ObjectList<Normal> const& objectList, departures | boost::adaptors::map_values) {
            foreach(const Box& box, objectList.getList<Box>()) {
                if (temporalIntersectingExclusive(protoButtons[i], buttonPositions[i], box)) {
                    goto intersecting;
                }
            }
            foreach(const Guy& guy, objectList.getList<Guy>()) {
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

mt::boost::container::vector<RectangleGlitz>::type calculateButtonGlitz(
    boost::container::vector<ProtoButton> const& protoButtons,
    mt::boost::container::vector<char>::type const& buttonStates,
    mt::boost::container::vector<PositionAndVelocity2D>::type const& buttonStore)
{
    assert(boost::distance(protoButtons) == boost::distance(buttonStates));
    assert(boost::distance(protoButtons) == boost::distance(buttonStore));
    mt::boost::container::vector<RectangleGlitz>::type retv;
    retv.reserve(boost::distance(protoButtons));
    for (std::size_t i(0), end(boost::distance(protoButtons)); i != end; ++i) {
        unsigned colour(buttonStates[i]?0x96FF9600u:0xFF969600u);
        retv.push_back(
            RectangleGlitz(
                buttonStore[i].getX(), buttonStore[i].getY(),
                protoButtons[i].width_, protoButtons[i].height_,
                buttonStore[i].getXspeed(), buttonStore[i].getYspeed(),
                colour, colour,
                protoButtons[i].timeDirection_));
    }
    return retv;
}

void fillButtonTriggers(
    mt::boost::container::vector<mt::boost::container::vector<int>::type>::type& triggers,
    boost::container::vector<ProtoButton> const& protoButtons,
    mt::boost::container::vector<char>::type const& states)
{
    for (std::size_t i(0), end(boost::distance(protoButtons)); i != end; ++i)
    {
        triggers[protoButtons[i].triggerID_].push_back(states[i]);
    }
}

mt::boost::container::map<Frame*, mt::boost::container::vector<TriggerData>::type >::type calculateActualTriggerDepartures(
    BOOST_RV_REF(mt::boost::container::vector<mt::boost::container::vector<int>::type>::type) triggers,
    boost::container::vector<std::pair<int, boost::container::vector<int> > > const& triggerOffsetsAndDefaults,
    Frame* currentFrame)
{
    assert(boost::distance(triggers) == boost::distance(triggerOffsetsAndDefaults));
    mt::boost::container::map<Frame*, mt::boost::container::vector<TriggerData>::type >::type retv;
    Universe& universe(getUniverse(currentFrame));
    for (std::size_t i(0), end(boost::distance(triggers)); i != end; ++i)
    {
        retv[
            getArbitraryFrame(
                universe, 
                getFrameNumber(currentFrame) + triggerOffsetsAndDefaults[i].first)].push_back(
                    TriggerData(i, boost::move(triggers[i])));
    }
    return retv;
}

}//namespace


std::pair<
    mt::boost::container::map<Frame*, mt::boost::container::vector<TriggerData>::type >::type,
    mt::boost::container::vector<RectangleGlitz>::type
> 
NewOldTriggerFrameState::getTriggerDeparturesAndGlitz(
    mt::boost::container::map<Frame*, ObjectList<Normal> >::type const& departures,
    Frame* currentFrame)
{
    //These are the states that are apparent in the buttonGlitz
    //These are also the states that travel through time to affect
    //platforms at their nextframes.
    mt::boost::container::vector<char>::type buttonStates(calculateButtonStates(triggerSystem.protoButtons_, buttonStore, departures));

    boost::push_back(glitzStore, calculateButtonGlitz(triggerSystem.protoButtons_, buttonStates, buttonStore));
    
    fillButtonTriggers(triggers, triggerSystem.protoButtons_, buttonStates);
    
    return std::make_pair(calculateActualTriggerDepartures(boost::move(triggers), triggerSystem.triggerOffsetsAndDefaults, currentFrame), glitzStore);
}

NewOldTriggerFrameState::NewOldTriggerFrameState(NewOldTriggerSystem const& triggerSys) :
    buttonStore(),
    triggers(boost::distance(triggerSys.triggerOffsetsAndDefaults)),
    glitzStore(),
    triggerSystem(triggerSys)
{
}

}//namespace hg
