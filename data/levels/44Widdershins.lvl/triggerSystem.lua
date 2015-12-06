local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {xOffset = 13 * 3200, yOffset = 4 * 3200},
            index = 1,
            width = 2 * 3200,
            height = 2 * 3200,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = 1,
            xDestination = 0,
            yDestination = 0,
            relativeTime = false,
            timeDestination = 0,
            illegalDestination = 1,
            fallable = false,
            winner = true
        },
    },
    protoCollisions = {
        {
            width = 2 * 3200,
            height = 1 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 2,
            buttonTriggerID = 1,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 16 * 3200,
                        maxSpeed = 200,
                        acceleration = 20,
                        deceleration = 20
                    },
                    yDestination = {
                        desiredPosition = 6 * 3200,
                        maxSpeed = 300,
                        acceleration = 20,
                        deceleration = 20
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 21 * 3200,
                        maxSpeed = 200,
                        acceleration = 20,
                        deceleration = 20
                    },
                    yDestination = {
                        desiredPosition = 6 * 3200,
                        maxSpeed = 300,
                        acceleration = 20,
                        deceleration = 20
                    }
                }
            }
        },
    },
    protoMutators = {
        bts.pickup{
            attachment = {platform = nil, xOffset = 13.5 * 3200, yOffset = 9 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 3,
            pickupType = 'timeJump'
        },
        bts.pickup{
            attachment = {platform = nil, xOffset = 14.5 * 3200, yOffset = 9 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 4,
            pickupType = 'timeGun'
        },
        bts.pickup{
            attachment = {platform = nil, xOffset = 15.5 * 3200, yOffset = 9 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 5,
            pickupType = 'timeGun'
        },
    },
    protoButtons = {
        bts.stickySwitch{
            attachment = {xOffset = 29.75 * 3200, yOffset = 6 * 3200},
            width = 800,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 1,
        },
    },
    protoGlitz = {
    },
}

calculatePhysicsAffectingStuff = bts.calculatePhysicsAffectingStuff(tempStore)

function shouldArrive(dynamicObject)
    return true
end
function shouldPort(responsiblePortalIndex, dynamicObject, porterActionedPortal)
    return true
end
mutateObject = bts.mutateObject(tempStore)

getDepartureInformation = bts.getDepartureInformation(tempStore)
