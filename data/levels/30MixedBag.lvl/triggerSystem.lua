local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 3.5 * 3200, yOffset = 15 * 3200},
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
        }
    },
    protoCollisions = {
        {
            width = 2 * 3200,
            height = 1 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 3,
            buttonTriggerID = 2,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 18 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 5.5 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 18 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 9 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                }
            }
        }
    },
    protoMutators = {
        bts.pickup{
            attachment = {platform = nil, xOffset = 17.75 * 3200, yOffset = 16.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 1,
            pickupType = 'timeJump'
        },
        bts.pickup{
            attachment = {platform = nil, xOffset = 18.75 * 3200, yOffset = 16.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 4,
            pickupType = 'timeGun'
        },
        bts.pickup{
            attachment = {platform = nil, xOffset = 19.75 * 3200, yOffset = 16.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 5,
            pickupType = 'timeReverse'
        },
    },
    protoButtons = {
        bts.momentarySwitch{
            attachment = {platform = nil, xOffset = 13 * 3200, yOffset = 16 * 3200},
            width = 800,
            height = 3200,
            timeDirection = 'forwards',
            triggerID = 2
        }
    }
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
