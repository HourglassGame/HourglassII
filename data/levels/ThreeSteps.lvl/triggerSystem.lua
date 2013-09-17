local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 23 * 3200, yOffset = 2 * 3200},
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
            width = 3200,
            height = 0.5 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 5,
			triggerFunction = function(triggers)
				return triggers[1][1] == 1 and triggers[2][1] == 1
			end,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 21 * 3200,
                        maxSpeed = 300,
                        acceleration = 25,
                        deceleration = 25
                    },
                    yDestination = {
                        desiredPosition = 4 * 3200,
                        maxSpeed = 300,
                        acceleration = 25,
                        deceleration = 25
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 21 * 3200,
                        maxSpeed = 300,
                        acceleration = 25,
                        deceleration = 25
                    },
                    yDestination = {
                        desiredPosition = 12.5 * 3200,
                        maxSpeed = 300,
                        acceleration = 25,
                        deceleration = 25
                    }
                }
            }
        }
    },
    protoMutators = {
        bts.pickup{
            attachment = {platform = nil, xOffset = 8.25 * 3200, yOffset = 6 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 3,
			pickupType = 'timeJump'
        },
		bts.pickup{
            attachment = {platform = nil, xOffset = 18.25 * 3200, yOffset = 12 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 4,
			pickupType = 'timeGun'
        }
    },
    protoButtons = {
        bts.momentarySwitch{
            attachment = {platform = nil, xOffset = 8 * 3200, yOffset = 6.75 * 3200},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 1
        },
		bts.momentarySwitch{
            attachment = {platform = nil, xOffset = 13 * 3200, yOffset = 9.75 * 3200},
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
