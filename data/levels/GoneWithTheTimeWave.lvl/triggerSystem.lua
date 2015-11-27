local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {xOffset = 21.25 * 3200, yOffset = 7 * 3200},
            index = 1,
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
            attachment = {platform = 2, xOffset = 0 * 3200, yOffset = -4 * 3200},
            index = 2,
            width = 1600,
            height = 9600,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = 0,
            xDestination = 0,
            yDestination = 0,
            relativeTime = true,
            timeDestination = -600,
            illegalDestination = 2,
            fallable = true,
            winner = false,
            triggerFunction = function (triggers)
                return triggers[1][1] == 1
            end,
        }
    },
    protoCollisions = {
        {
            width = 1 * 3200,
            height = 3 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 2,
            buttonTriggerID = 1,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 19.5 * 3200,
                        maxSpeed = 200,
                        acceleration = 20,
                        deceleration = 20
                    },
                    yDestination = {
                        desiredPosition = 3 * 3200,
                        maxSpeed = 300,
                        acceleration = 20,
                        deceleration = 20
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 19.5 * 3200,
                        maxSpeed = 200,
                        acceleration = 20,
                        deceleration = 20
                    },
                    yDestination = {
                        desiredPosition = 6 * 3200,
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
				if triggerArrivals[1][1] == 0 and triggerArrivals[4][1] == 0 then
					return {
						x = 19.75 * 3200,
						y = 10 * 3200,
						xspeed = 0,
						yspeed = 0,
						width = 1600,
						height = 2000,
						timeDirection = 'forwards'
					}
				end
				
				-- 600 frames to sweep map (10 seconds)
				local x = triggerArrivals[3][1]
				x = x - 128
				if x < 1600 then
					x = x + 24 * 3200
				end
				return {
						x = x,
						y = 10 * 3200,
						xspeed = -128,
						yspeed = 0,
						width = 1500,
						height = 2000,
						timeDirection = 'forwards'
					}
				
			end
        },
    },
	protoMutators = {
	},
    protoButtons = {
		bts.stickySwitch{
            attachment = {xOffset = 9.25 * 3200, yOffset = 8.75 * 3200},
            width = 1600,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 1,
            extraTriggerIDs = {4}
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
