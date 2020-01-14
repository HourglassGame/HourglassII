local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 2.5 * 3200, yOffset = 3 * 3200},
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
        {
            attachment = {platform = 1, xOffset = 11.5 * 3200, yOffset = 4.5 * 3200},
            index = 2,
            width = 2 * 3200,
            height = 2 * 3200,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = 2,
            xDestination = 0,
            yDestination = 0,
            relativeTime = false,
            timeDestination = 0,
            illegalDestination = 2,
            fallable = false,
            winner = false
        },
    },
    protoCollisions = {
        {
            width = 0.5 * 3200,
            height = 0.5 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 4,
            buttonTriggerID = 3,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 0 * 3200,
                        maxSpeed = 250,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 9.5 * 3200,
                        maxSpeed = 250,
                        acceleration = 50,
                        deceleration = 50
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 0 * 3200,
                        maxSpeed = 250,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 0 * 3200,
                        maxSpeed = 250,
                        acceleration = 50,
                        deceleration = 50
                    }
                }
            }
        },
    },
    protoMutators = {
        bts.pickup{
            attachment = {platform = nil, xOffset = 16.25 * 3200, yOffset = 15.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 1,
            pickupType = 'timeReverse'
        },
        bts.pickup{
            attachment = {platform = nil, xOffset = 17.25 * 3200, yOffset = 15.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 2,
            pickupType = 'timeReverse'
        },
    },
    protoButtons = {
        bts.momentarySwitch{
            attachment = {platform = nil, xOffset = 27.5 * 3200, yOffset = 11.75 * 3200},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 3
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
