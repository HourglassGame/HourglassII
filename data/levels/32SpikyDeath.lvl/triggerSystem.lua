local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 15 * 3200, yOffset = 5 * 3200},
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
            width = 16 * 3200,
            height = 0.5 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 3,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 9 * 3200,
                        maxSpeed = 110,
                        acceleration = 18,
                        deceleration = 18
                    },
                    yDestination = {
                        desiredPosition = 10 * 3200,
                        maxSpeed = 110,
                        acceleration = 18,
                        deceleration = 18
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 9 * 3200,
                        maxSpeed = 110,
                        acceleration = 18,
                        deceleration = 18
                    },
                    yDestination = {
                        desiredPosition = 10 * 3200,
                        maxSpeed = 110,
                        acceleration = 18,
                        deceleration = 18
                    }
                }
            }
        },
        {
            width = 16 * 3200,
            height = 1 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 4,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 9 * 3200,
                        maxSpeed = 110,
                        acceleration = 18,
                        deceleration = 18
                    },
                    yDestination = {
                        desiredPosition = 10 * 3200,
                        maxSpeed = 110,
                        acceleration = 18,
                        deceleration = 18
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 9 * 3200,
                        maxSpeed = 110,
                        acceleration = 18,
                        deceleration = 18
                    },
                    yDestination = {
                        desiredPosition = 10 * 3200,
                        maxSpeed = 120,
                        acceleration = 18,
                        deceleration = 110
                    }
                }
            }
        }
    },
    protoMutators = {
        bts.pickup{
            attachment = {platform = 2, xOffset = 5.75 * 3200, yOffset = -0.75 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 1,
            pickupType = 'timeJump'
        },
        bts.pickup{
            attachment = {platform = 2, xOffset = 7.75 * 3200, yOffset = -0.75  * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 2,
            pickupType = 'timeGun'
        },
        bts.spikes{
            attachment = {platform = 1, xOffset = 0 * 3200, yOffset = 0.5 * 3200},
            width = 14 * 3200,
            height = 0.5 * 3200,
            timeDirection = 'forwards',
        },
    },
    protoButtons = {
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
