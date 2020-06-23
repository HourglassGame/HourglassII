local bts = require "global.basicTriggerSystem"

local tempStore = 
{
    protoPortals =
    {
        {
            attachment = {platform = 1, xOffset = -4200, yOffset = -3200},
            index = 1,
            width = 4200,
            height = 4200,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = 1,
            xDestination = 0,
            yDestination = -16000,
            relativeTime = true,
            timeDestination = 120,
            illegalDestination = 1,
            fallable = true,
            winner = false,
        }
    },
    protoCollisions = {
        {
            width = 6400,
            height = 1600,
            timeDirection = 'forwards',
            lastStateTriggerID = 2,
            buttonTriggerID = 1,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 22400,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 43800,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 38400,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 43800,
                        maxSpeed = 300,
                        acceleration = 20,
                        deceleration = 18
                    }
                }
            }
        }
    },
    protoMutators = {
    },
    protoButtons = {
        bts.momentarySwitch{
            attachment = {platform = 1, xOffset = 3200, yOffset = -800},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 1
        },
        bts.momentarySwitch{
            attachment = {platform = nil, xOffset = 3200, yOffset = 37600},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 3
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

