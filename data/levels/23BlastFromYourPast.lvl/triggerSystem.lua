local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 8.5 * 3200, yOffset = 12 * 3200},
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
            winner = false
        },
        {
            attachment = {platform = nil, xOffset = 4.5 * 3200, yOffset = 7 * 3200},
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
            width = 3 * 3200,
            height = 1 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 3,
            buttonTriggerID = 2,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 12 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 15 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 12 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 14.5 * 3200,
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
            attachment = {platform = nil, xOffset = 11.25 * 3200, yOffset = 13.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 1,
            pickupType = 'timeGun'
        },
        bts.spikes{
            attachment = {platform = 1, xOffset = 1 * 3200, yOffset = -1 * 3200},
            width = 9 * 3200,
            height = 0.5 * 3200,
            timeDirection = 'forwards',
        },
    },
    protoButtons = {
        bts.momentarySwitch{
            attachment = {platform = nil, xOffset = 24 * 3200, yOffset = 13.75 * 3200},
            width = 3200,
            height = 800,
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
