#ifndef HG_PROTO_STUFF_H
#define HG_PROTO_STUFF_H
#include "PlatformDestination.h"
#include "Collision.h"
#include "PortalArea.h"
#include "AttachmentMap.h"
#include "mt/std/vector"

namespace hg {
struct PositionAndVelocity2D {
    PositionAndVelocity2D(int x, int y, int xSpeed, int ySpeed) :
    xPositionAndVelocity(x, xSpeed),
    yPositionAndVelocity(y, ySpeed)
    {
    }
    PositionAndVelocity xPositionAndVelocity;
    PositionAndVelocity yPositionAndVelocity;
    
    int getX() const { return xPositionAndVelocity.position; }
    int getY() const { return yPositionAndVelocity.position; }
    int getXspeed() const{ return xPositionAndVelocity.velocity; }
    int getYspeed() const{ return yPositionAndVelocity.velocity; }
    
};
//The stuff needed to create a PortalArea, given trigger arrivals
struct ProtoPortal {
    PortalArea calculatePortalArea(mt::std::vector<Collision>::type const&) const;
    ProtoPortal(
        Attachment const& attachment,
        int width,
        int height,
        TimeDirection timeDirection,
        int destinationIndex,
        int xDestination,
        int yDestination,
        bool relativeTime,
        int timeDestination,
        int illegalDestination,
        bool fallable,
        bool winner) :
    attachment_(attachment),
    width_(width),
    height_(height),
    timeDirection_(timeDirection),
    destinationIndex_(destinationIndex),
    xDestination_(xDestination),
    yDestination_(yDestination),
    relativeTime_(relativeTime),
    timeDestination_(timeDestination),
    illegalDestination_(illegalDestination),
    fallable_(fallable),
    winner_(winner)
    {}
private:
    //attachment_ has a dual meaning.
    //If attachment_.platformIndex == std::numeric_limits<std::size_t>::max()
    //then attachment_.xOffset and atachment_.yOffset give the absolute position of the portal.
    //Otherwise they give the position relative to the platform with index == attachment_.platformIndex
    Attachment attachment_;    
    int width_;
    int height_;
    TimeDirection timeDirection_;
    int destinationIndex_;
    int xDestination_;
    int yDestination_;
    bool relativeTime_;
    int timeDestination_;
    int illegalDestination_;
    bool fallable_;
    bool winner_;
};
//The stuff needed to create a CollisionArea, given trigger arrivals
//Also used to create the trigger departure relevant to the platform.
struct ProtoPlatform {
    ProtoPlatform(
        int width,
        int height,
        TimeDirection timeDirection,
        int lastStateTriggerID,
        int buttonTriggerID,
        PlatformDestinationPair const& destinations) :
    width_(width),
    height_(height),
    timeDirection_(timeDirection),
    lastStateTriggerID_(lastStateTriggerID),
    buttonTriggerID_(buttonTriggerID),
    destinations_(destinations)
    {
    }

    //uses trigger arrivals to generate Collision
    Collision calculateCollision(
            mt::std::vector<mt::std::vector<int>::type>::type const&) const;
    int width_;
    int height_;
    TimeDirection timeDirection_;
    //index of the trigger that holds [last-x-position, last-y-position, x-speed, y-speed]
    int lastStateTriggerID_;
    //index of the trigger that holds [on/off]
    //this is shared with the button controlling the platform.
    //The button writes to the trigger, the platform reads from the trigger
    int buttonTriggerID_;
    PlatformDestinationPair destinations_;
};

//Used to create the rectangle glitz for the button, as well as the trigger departure for the button.
struct ProtoButton {
    ProtoButton(
        Attachment const& attachment,
        int width,
        int height,
        TimeDirection timeDirection,
        int triggerID) :
    attachment_(attachment),
    width_(width),
    height_(height),
    timeDirection_(timeDirection),
    triggerID_(triggerID)
    {}


    PositionAndVelocity2D calculatePositionAndVelocity2D(
            mt::std::vector<Collision>::type const& collisions) const;
    //See comment in ProtoPortal that explains the meaning of attachment_.
    Attachment attachment_;
    int width_;
    int height_;
    TimeDirection timeDirection_;
    //index of the trigger to which this button writes [on/off]
    int triggerID_;
};

}
#endif //HG_PROTO_STUFF_H
