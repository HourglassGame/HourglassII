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
            width = 3 * 3200,
            height = 5 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 2,
            buttonTriggerID = 1,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 110 * 3200,
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
                        desiredPosition = 110 * 3200,
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
				if triggerArrivals[1][1] == 1 then
					return
					{
						x = 2 * 3200,
						y = 3 * 3200,
						xspeed = 0,
						yspeed = 0,
						width = 2 * 3200,
						height = 0.5 * 3200,
						timeDirection = 'forwards'
					}
				else
					return
					{
						x = 2 * 3200,
						y = 3 * 3200,
						xspeed = 0,
						yspeed = 0,
						width = 20 * 3200,
						height = 0.5 * 3200,
						timeDirection = 'forwards'
					}
				
				end
				
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
