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
		{
            attachment = {xOffset = 11.5 * 3200, yOffset = 12.5 * 3200},
            index = 2,
            width = 6400,
            height = 6400,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = 3,
            xDestination = 0,
            yDestination = 0,
            relativeTime = true,
            timeDestination = -300,
            illegalDestination = 3,
            fallable = true,
            winner = false,
        },
		{
            attachment = {xOffset = 11.5 * 3200, yOffset = 2.5 * 3200},
            index = 3,
            width = 6400,
            height = 6400,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = 2,
            xDestination = 0,
            yDestination = 0,
            relativeTime = true,
            timeDestination = 300,
            illegalDestination = 2,
            fallable = true,
            winner = false,
        }
    },
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
                        desiredPosition = 11 * 3200,
                        maxSpeed = 200,
                        acceleration = 20,
                        deceleration = 20
                    },
                    yDestination = {
                        desiredPosition = 14.5 * 3200,
                        maxSpeed = 300,
                        acceleration = 20,
                        deceleration = 20
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 11 * 3200,
                        maxSpeed = 200,
                        acceleration = 20,
                        deceleration = 20
                    },
                    yDestination = {
                        desiredPosition = 11 * 3200,
                        maxSpeed = 300,
                        acceleration = 20,
                        deceleration = 20
                    }
                }
            }
        },
		{
            lastStateTriggerID = 3,
			rawCollisionFunction = function(triggerArrivals, outputTriggers)
			
				return
				{
					x = 25000,
					y = 25000,
					xspeed = 0,
					yspeed = 0,
					width = 5000,
					height = 6000,
					timeDirection = 'forwards'
				}
			end
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
