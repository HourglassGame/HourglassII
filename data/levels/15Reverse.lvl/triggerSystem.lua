local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 27.5 * 3200, yOffset = 8 * 3200},
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
            width = 4 * 3200,
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
                        acceleration = 10,
                        deceleration = 10
                    },
                    yDestination = {
                        desiredPosition = 17 * 3200,
                        maxSpeed = 200,
                        acceleration = 10,
                        deceleration = 10
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 18 * 3200,
                        maxSpeed = 200,
                        acceleration = 10,
                        deceleration = 10
                    },
                    yDestination = {
                        desiredPosition = 10 * 3200,
                        maxSpeed = 200,
                        acceleration = 10,
                        deceleration = 10
                    }
                }
            }
        }
    },
    protoMutators = {
        bts.pickup{
            attachment = {platform = nil, xOffset = 8.75 * 3200, yOffset = 8.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 1,
            pickupType = 'timeReverse'
        },
    },
    protoButtons = {
        bts.stickySwitch{
            attachment = {platform = nil, xOffset = 8 * 3200, yOffset = 12 * 3200},
            width = 800,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 2
        },
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
