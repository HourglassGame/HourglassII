local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 18 * 3200, yOffset = 5 * 3200},
            index = 1,
            width = 2 * 3200,
            height = 2 * 3200,
            collisionOverlap = 50,
            timeDirection = 'reverse',
            destinationIndex = 1,
            xDestination = 0,
            yDestination = 0,
            relativeTime = false,
            timeDestination = 0,
            illegalDestination = 1,
            fallable = false,
            winner = true
        }
    },
    protoCollisions = {
        {
            width = 3200,
            height = 4 * 3200,
            timeDirection = 'reverse',
            lastStateTriggerID = 2,
            buttonTriggerID = 1,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 16 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = -1 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 16 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 3 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                }
            }
        }
    },
	protoBoxCreators = {
		bts.boxOMatic{
			attachment = {platform = nil, xOffset = 14 * 3200, yOffset = 1.25 * 3200},
			width = 3200,
			height = 4800,
			timeDirection = 'forwards',
			boxCreationFunction = function (triggers, frameNumber)
				if frameNumber == 0 and triggers[1][1] == 0 then
					return {
						x = 0,
						y = 800,
						xspeed = -1100,
						yspeed = 0,
						size = 3200,
						timeDirection = 'forwards',
					}
				else
					return false
				end
			end,
		}
	},
    protoMutators = {
    },
    protoButtons = {
        bts.multiStickySwitch{
            buttons = {
				{
					attachment = {platform = nil, xOffset = 4.15 * 3200, yOffset = 6.75 * 3200},
					width = 1600,
					height = 800,
				},
				{
					attachment = {platform = nil, xOffset = 6.55 * 3200, yOffset = 6.75 * 3200},
					width = 1600,
					height = 800,
				},
				{
					attachment = {platform = nil, xOffset = 8.95 * 3200, yOffset = 6.75 * 3200},
					width = 1600,
					height = 800,
				},
				{
					attachment = {platform = nil, xOffset = 11.35 * 3200, yOffset = 6.75 * 3200},
					width = 1600,
					height = 800,
				},
			},
            timeDirection = 'reverse',
            triggerID = 1,
            stateTriggerID = 3
        },
    },
	protoGlitz = {
		bts.wireGlitz{
			x1 = {platform = nil, pos = 4.15 * 3200},
			y1 = {platform = nil, pos = 7 * 3200 },
			x2 = {platform = nil, pos = 11.85 * 3200},
			y2 = {platform = nil, pos = 7.2 * 3200},
			triggerID = 1,
			useTriggerArrival = false
		},
		bts.wireGlitz{
			x1 = {platform = nil, pos = 8.2 * 3200},
			y1 = {platform = nil, pos = 7.2 * 3200 },
			x2 = {platform = nil, pos = 8.3 * 3200},
			y2 = {platform = nil, pos = 7.55 * 3200},
			triggerID = 1,
			useTriggerArrival = false
		},
		bts.wireGlitz{
			x1 = {platform = nil, pos = 21.45 * 3200},
			y1 = {platform = nil, pos = 7.45 * 3200 },
			x2 = {platform = nil, pos = 8.3 * 3200},
			y2 = {platform = nil, pos = 7.55 * 3200},
			triggerID = 1,
			useTriggerArrival = false
		},
		bts.wireGlitz{
			x1 = {platform = nil, pos = 21.45 * 3200},
			y1 = {platform = nil, pos = 2.45 * 3200 },
			x2 = {platform = nil, pos = 21.55 * 3200},
			y2 = {platform = nil, pos = 7.55 * 3200},
			triggerID = 1,
			useTriggerArrival = false
		},
		bts.wireGlitz{
			x1 = {platform = nil, pos = 16.45 * 3200},
			y1 = {platform = nil, pos = 2.45 * 3200 },
			x2 = {platform = nil, pos = 21.55 * 3200},
			y2 = {platform = nil, pos = 2.55 * 3200},
			triggerID = 1,
			useTriggerArrival = false
		},
		bts.wireGlitz{
			x1 = {platform = nil, pos = 16.45 * 3200},
			y1 = {platform = nil, pos = 3 * 3200 },
			x2 = {platform = nil, pos = 16.55 * 3200},
			y2 = {platform = nil, pos = 2.55 * 3200},
			triggerID = 1,
			useTriggerArrival = false
		},
		bts.wireGlitz{
			x1 = {platform = nil, pos = 16.45 * 3200},
			y1 = {platform = nil, pos = 1.95 * 3200 },
			x2 = {platform = nil, pos = 16.55 * 3200},
			y2 = {platform = nil, pos = 2.45 * 3200},
			triggerFunction = function (triggerArrivals, outputTriggers)
				return triggerArrivals[1][1] == 0
			end,
		},
		bts.wireGlitz{
			x1 = {platform = nil, pos = 16.45 * 3200},
			y1 = {platform = nil, pos = 1.95 * 3200 },
			x2 = {platform = nil, pos = 15 * 3200},
			y2 = {platform = nil, pos = 2.05 * 3200},
			triggerFunction = function (triggerArrivals, outputTriggers)
				return triggerArrivals[1][1] == 0
			end,
		},
		bts.basicTextGlitz{
			x = 15 * 3200,
			y = 1.25 * 3200,
			text = " <- Creates a box at the start\n                if the door is closed.",
			size = 1400,
			layer = 1600,
			colour = {r = 255, g = 255, b = 255},
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
