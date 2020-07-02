local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 4 * 3200, yOffset = 7 * 3200},
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
            winner = true,
        },
        --{
        --    attachment = {platform = nil, xOffset = 10 * 3200, yOffset = 7 * 3200},
        --    index = 2,
        --    width = 2 * 3200,
        --    height = 2 * 3200,
        --    collisionOverlap = 50,
        --    timeDirection = 'forwards',
        --    destinationIndex = 2,
        --    xDestination = 0,
        --    yDestination = 0,
        --    relativeTime = false,
        --    timeDestination = 0,
        --    illegalDestination = 2,
        --    fallable = false,
        --    winner = false
        --},
    },
    protoCollisions = {
        {
            width = 1 * 3200,
            height = 3 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 1,
            triggerClause = "0",
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 7 * 3200,
                        maxSpeed = 300,
                        acceleration = 16,
                        deceleration = 16
                    },
                    yDestination = {
                        desiredPosition = 6 * 3200,
                        maxSpeed = 300,
                        acceleration = 16,
                        deceleration = 16
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 7 * 3200,
                        maxSpeed = 300,
                        acceleration = 16,
                        deceleration = 16
                    },
                    yDestination = {
                        desiredPosition = 6 * 3200,
                        maxSpeed = 300,
                        acceleration = 16,
                        deceleration = 16
                    }
                }
            }
        }
    },
    protoMutators = {
        bts.pickup{
            attachment = {platform = nil, xOffset = 20.75 * 3200, yOffset = 11 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 2,
            pickupType = 'timeReverse'
        },
        bts.pickup{
            attachment = {platform = nil, xOffset = 24.75 * 3200, yOffset = 13 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 3,
            pickupType = 'timeJump'
        }
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
