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
            destinationIndex = nil,
            xDestination = 0,
            yDestination = 0,
            relativeTime = false,
            timeDestination = 0,
            illegalDestination = 1,
            fallable = false,
            winner = true
        },
        {
            attachment = {xOffset = 6 * 3200, yOffset = 6 * 3200},
            index = 2,
            xaim = 5 * 3200,
            yaim = 20 * 3200,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = nil,
            xDestination = 0,
            yDestination = 0,
            relativeTime = true,
            timeDestination = -600,
            illegalDestination = 1,
            fallable = false,
            winner = false,
            isLaser = true,
			triggerFunction = function(triggerArrivals, frameNumber)
				return triggerArrivals[1][1] == 1
			end
        },
    },
    --[[
    protoLasers =
    {
        {
            attachment = {xOffset = 6 * 3200, yOffset = 6 * 3200},
            index = 2,
            xaim = 5 * 3200,
            yaim = 20 * 3200,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = nil,
            xDestination = 0,
            yDestination = 0,
            relativeTime = true,
            timeDestination = -600,
            illegalDestination = 1,
            fallable = false,
            winner = false,
			triggerFunction = function(triggerArrivals, frameNumber)
				return triggerArrivals[1][1] == 1
			end
        },
    },
    ]]--
    protoCollisions = {
        {
            width = 3 * 3200,
            height = 5 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 2,
            buttonTriggerID = 1,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 12 * 3200,
                        maxSpeed = 200,
                        acceleration = 20,
                        deceleration = 20
                    },
                    yDestination = {
                        desiredPosition = 12 * 3200,
                        maxSpeed = 300,
                        acceleration = 20,
                        deceleration = 20
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 12 * 3200,
                        maxSpeed = 200,
                        acceleration = 20,
                        deceleration = 20
                    },
                    yDestination = {
                        desiredPosition = 12 * 3200,
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
        bts.momentarySwitch{
            attachment = {xOffset = 7 * 3200, yOffset = 10.75 * 3200},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 1,
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
