local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 24 * 3200, yOffset = 9 * 3200},
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
        {
            attachment = {platform = nil, xOffset = 26.5 * 3200, yOffset = 9 * 3200},
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
            width = 3200,
            height = 3 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 1,
            buttonTriggerID = 3,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 12.5 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 5 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 12.5 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 8 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                }
            }
        },
        {
            width = 3200,
            height = 3 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 2,
            triggerClause = "t4 t5 &",
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 22.5 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 5 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 22.5 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 8 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                }
            }
        },
    },
    protoMutators = {
        bts.pickup{
            attachment = {platform = nil, xOffset = 20.25 * 3200, yOffset = 10.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 6,
            pickupType = 'timeJump'
        },
        bts.pickup{
            attachment = {platform = nil, xOffset = 4.75 * 3200, yOffset = 12.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 7,
            pickupType = 'timeJump'
        },
        bts.pickup{
            attachment = {platform = nil, xOffset = 8.75 * 3200, yOffset = 12.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 8,
            pickupType = 'timeJump'
        },
    },
    protoButtons = {
        bts.momentarySwitch{
            attachment = {platform = nil, xOffset = 17.5 * 3200, yOffset = 10.75 * 3200},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 3
        },
        bts.momentarySwitch{
            attachment = {platform = nil, xOffset = 4.5 * 3200, yOffset = 13.75 * 3200},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 4
        },
        bts.momentarySwitch{
            attachment = {platform = nil, xOffset = 8.5 * 3200, yOffset = 13.75 * 3200},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 5
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
