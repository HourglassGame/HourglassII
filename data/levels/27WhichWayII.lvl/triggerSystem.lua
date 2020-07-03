local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {xOffset = 15 * 3200, yOffset = 13.5 * 3200},
            index = 1,
            width = 2 * 3200,
            height = 2 * 3200,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = nil,
            xDestination = 0,
            yDestination = 0,
            relativeTime = false,
            timeDestination = 0,
            illegalDestination = nil,
            fallable = false,
            winner = true
        },
    },
    protoCollisions = {
        {
            width = 2 * 3200,
            height = 2 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 3,
            triggerClause = "t1 t2 &",
            --triggerFunction = function (triggers)
            --    return triggers[1][1] > 0 and triggers[2][1] > 0
            --end,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 15 * 3200,
                        maxSpeed = 200,
                        acceleration = 20,
                        deceleration = 18
                    },
                    yDestination = {
                        desiredPosition = 17 * 3200,
                        maxSpeed = 300,
                        acceleration = 20,
                        deceleration = 18
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 15 * 3200,
                        maxSpeed = 200,
                        acceleration = 20,
                        deceleration = 18
                    },
                    yDestination = {
                        desiredPosition = 11 * 3200,
                        maxSpeed = 300,
                        acceleration = 20,
                        deceleration = 18
                    }
                }
            }
        },
    },
    protoMutators = {
        bts.pickup{
            attachment = {platform = nil, xOffset = 4.25 * 3200, yOffset = 12.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 4,
            pickupType = 'timeJump'
        },
        bts.pickup{
            attachment = {platform = nil, xOffset = 15.25 * 3200, yOffset = 10.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 5,
            pickupType = 'timeJump'
        },
        bts.pickup{
            attachment = {platform = nil, xOffset = 16.25 * 3200, yOffset = 10.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 6,
            pickupType = 'timeReverse'
        },
        bts.pickup{
            attachment = {platform = nil, xOffset = 27.25 * 3200, yOffset = 12.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 7,
            pickupType = 'timeReverse'
        },
    },
    protoButtons = {
        bts.momentarySwitch{
            attachment = {xOffset = 2 * 3200, yOffset = 12.75 * 3200},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 1,
        },
        bts.momentarySwitch{
            attachment = {xOffset = 29 * 3200, yOffset = 12.75 * 3200},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 2,
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
