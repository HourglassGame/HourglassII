local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {xOffset = 21.5 * 3200, yOffset = 5 * 3200},
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
            illegalDestination = 1,
            fallable = false,
            winner = true
        },
    },
    protoCollisions = {
        {
            width = 6 * 3200,
            height = 1 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 2,
            buttonTriggerID = 1,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 15 * 3200,
                        maxSpeed = 200,
                        acceleration = 20,
                        deceleration = 20
                    },
                    yDestination = {
                        desiredPosition = 8 * 3200,
                        maxSpeed = 300,
                        acceleration = 20,
                        deceleration = 20
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 15 * 3200,
                        maxSpeed = 200,
                        acceleration = 20,
                        deceleration = 20
                    },
                    yDestination = {
                        desiredPosition = 15 * 3200,
                        maxSpeed = 300,
                        acceleration = 20,
                        deceleration = 20
                    }
                }
            }
        },
    },
	protoMutators = {
	},
    protoButtons = {
        bts.toggleSwitch{
            triggerID = 1,
            timeDirection = 'forwards',
            first = {
                attachment = {platform = nil, xOffset = 20.75 * 3200, yOffset = 14.25 * 3200},
                width = 800,
                height = 1600,
            },
            second = {
                attachment = {platform = nil, xOffset = 20.75 * 3200, yOffset = 7.25 * 3200},
                width = 800,
                height = 1600,
            }
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
