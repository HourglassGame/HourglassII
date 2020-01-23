local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 27 * 3200, yOffset = 5 * 3200},
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
        }
    },
    protoCollisions = {
        {
            width = 2 * 3200,
            height = 0.5 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 5,
            --triggerFunction = function(triggers)
            --    return triggers[1][1] > 0 and triggers[2][1] > 0
            --end,
            triggerClause = "t1 t2 &",
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 23 * 3200,
                        maxSpeed = 300,
                        acceleration = 16,
                        deceleration = 16
                    },
                    yDestination = {
                        desiredPosition = 7 * 3200,
                        maxSpeed = 300,
                        acceleration = 16,
                        deceleration = 16
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 23 * 3200,
                        maxSpeed = 300,
                        acceleration = 16,
                        deceleration = 16
                    },
                    yDestination = {
                        desiredPosition = 15.5 * 3200,
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
            attachment = {platform = nil, xOffset = 10.25 * 3200, yOffset = 9 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 3,
            pickupType = 'timeJump'
        },
        bts.pickup{
            attachment = {platform = nil, xOffset = 20.25 * 3200, yOffset = 15 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 4,
            pickupType = 'timeGun'
        }
    },
    protoButtons = {
        bts.momentarySwitch{
            attachment = {platform = nil, xOffset = 10 * 3200, yOffset = 9.75 * 3200},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 1
        },
        bts.momentarySwitch{
            attachment = {platform = nil, xOffset = 15 * 3200, yOffset = 12.75 * 3200},
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
