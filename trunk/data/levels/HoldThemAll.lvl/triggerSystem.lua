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
            timeDirection = 'forwards',
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
            timeDirection = 'forwards',
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
    protoMutators = {
    },
    protoButtons = {
        bts.multiStickySwitch{
            buttons = {
				{
					attachment = {platform = nil, xOffset = 3.25 * 3200, yOffset = 6.75 * 3200},
					width = 1600,
					height = 800,
				},
				{
					attachment = {platform = nil, xOffset = 5.25 * 3200, yOffset = 6.75 * 3200},
					width = 1600,
					height = 800,
				},
				{
					attachment = {platform = nil, xOffset = 7.25 * 3200, yOffset = 6.75 * 3200},
					width = 1600,
					height = 800,
				},
				{
					attachment = {platform = nil, xOffset = 9.25 * 3200, yOffset = 6.75 * 3200},
					width = 1600,
					height = 800,
				},
				{
					attachment = {platform = nil, xOffset = 11.25 * 3200, yOffset = 6.75 * 3200},
					width = 1600,
					height = 800,
				},
				{
					attachment = {platform = nil, xOffset = 13.25 * 3200, yOffset = 6.75 * 3200},
					width = 1600,
					height = 800,
				},
			},
            timeDirection = 'forwards',
            triggerID = 1,
            stateTriggerID = 3
        },
    },
	protoGlitz = {
		bts.wireGlitz{
			x1 = {platform = nil, pos = 3.25 * 3200},
			y1 = {platform = nil, pos = 7 * 3200 },
			x2 = {platform = nil, pos = 13.75 * 3200},
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
