#include "ProtoStuff.h"
namespace hg {
Collision ProtoPlatform::calculateCollision(
        mt::std::vector<
            mt::std::vector<int>::type
        >::type const& triggers) const
{
        PlatformDestination const& destination(
            triggers[buttonTriggerID_].front() ?
            destinations_.onDestination :
            destinations_.offDestination);
        
        mt::std::vector<int>::type const& lastStateTrigger(triggers[lastStateTriggerID_]);
        
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
namespace {
PositionAndVelocity2D snapAttachment(
    TimeDirection timeDirection,
    Attachment const& attachment,
    mt::std::vector<Collision>::type const& collisions)
{
    int x;
    int y;
    int xSpeed;
    int ySpeed;
    if (attachment.platformIndex != std::numeric_limits<std::size_t>::max()) {
        Collision const& collision(collisions[attachment.platformIndex]);
        if (collision.getTimeDirection() * timeDirection == FORWARDS) {
            x = collision.getX() + attachment.xOffset;
            y = collision.getY() + attachment.yOffset;
            xSpeed = collision.getXspeed();
            ySpeed = collision.getYspeed();
        }
        else {
            x = collision.getX() - collision.getXspeed() + attachment.xOffset;
            y = collision.getY() - collision.getYspeed() + attachment.yOffset;
            //Speed should be (departure location - arrival location), but unfortunately
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
        x = attachment.xOffset;
        y = attachment.yOffset;
        xSpeed = 0;
        ySpeed = 0;
    }
    return PositionAndVelocity2D(
        x,
        y,
        xSpeed,
        ySpeed);
}
}
PortalArea ProtoPortal::calculatePortalArea(
        mt::std::vector<Collision>::type const& collisions) const
{
    PositionAndVelocity2D pnv2D(snapAttachment(timeDirection_, attachment_, collisions));
    return PortalArea(
        index_,
        pnv2D.getX(),
        pnv2D.getY(),
        width_,
        height_,
        pnv2D.getXspeed(),
        pnv2D.getYspeed(),
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
        mt::std::vector<Collision>::type const& collisions) const
{
    return snapAttachment(timeDirection_, attachment_, collisions);
}
}
