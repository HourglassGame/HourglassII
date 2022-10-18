local bts = require "global.basicTriggerSystem"

local tempStore =
{
	protoPortals =
	{
		{
			attachment = {platform = nil, xOffset = 26.5 * 3200, yOffset = 9 * 3200},
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
		},
		{
			attachment = {platform = nil, xOffset = 12.75 * 3200, yOffset = 12.5 * 3200},
			index = 2,
			width = 1.5 * 3200,
			height = 1.5 * 3200,
			collisionOverlap = 50,
			timeDirection = 'forwards',
			destinationIndex = 2,
			xDestination = 0,
			yDestination = 0,
			relativeTime = false,
			timeDestination = 0,
			illegalDestination = 2,
			fallable = false,
			guyOnly = true,
			winner = false
		},
	},
	protoCollisions = {
		{
			width = 3200,
			height = 2 * 3200,
			timeDirection = 'forwards',
			lastStateTriggerID = 3,
			buttonTriggerID = 1,
			destinations =
			{
				onDestination = {
					xDestination = {
						desiredPosition = 21 * 3200,
						maxSpeed = 200,
						acceleration = 50,
						deceleration = 50
					},
					yDestination = {
						desiredPosition = 7 * 3200,
						maxSpeed = 300,
						acceleration = 50,
						deceleration = 50
					}
				},
				offDestination = {
					xDestination = {
						desiredPosition = 21 * 3200,
						maxSpeed = 200,
						acceleration = 50,
						deceleration = 50
					},
					yDestination = {
						desiredPosition = 9 * 3200,
						maxSpeed = 300,
						acceleration = 50,
						deceleration = 50
					}
				},
			}
		},
		{
			width = 2 * 3200,
			height = 3200,
			timeDirection = 'forwards',
			lastStateTriggerID = 4,
			buttonTriggerID = 2,
			destinations =
			{
				onDestination = {
					xDestination = {
						desiredPosition = 23.5 * 3200,
						maxSpeed = 200,
						acceleration = 50,
						deceleration = 5
					},
					yDestination = {
						desiredPosition = 11 * 3200,
						maxSpeed = 2200,
						acceleration = 800,
						deceleration = 80
					}
				},
				offDestination = {
					xDestination = {
						desiredPosition = 23.5 * 3200,
						maxSpeed = 200,
						acceleration = 50,
						deceleration = 50
					},
					yDestination = {
						desiredPosition = 6 * 3200,
						maxSpeed = 80,
						acceleration = 30,
						deceleration = 200
					}
				}
			}
		},
	},
	protoMutators = {
		bts.pickup{
			attachment = {platform = nil, xOffset = 9.25 * 3200, yOffset = 11 * 3200},
			width = 1600,
			height = 1600,
			timeDirection = 'forwards',
			triggerID = 5,
			pickupType = 'timeJump'
		},
		bts.pickup{
			attachment = {platform = nil, xOffset = 17.25 * 3200, yOffset = 11 * 3200},
			width = 1600,
			height = 1600,
			timeDirection = 'forwards',
			triggerID = 6,
			pickupType = 'timeJump'
		},
	},
	protoButtons = {
		bts.stickySwitch{
			attachment = {platform = nil, xOffset = 24.25 * 3200, yOffset = 5 * 3200},
			width = 1600,
			height = 800,
			timeDirection = 'forwards',
			triggerID = 1
		},
		bts.momentarySwitch{
			attachment = {platform = nil, xOffset = 6 * 3200, yOffset = 13.75 * 3200},
			width = 15.25 * 3200,
			height = 800,
			timeDirection = 'forwards',
			triggerID = 2
		},
	},
	protoGlitz = {
		bts.basicRectangleGlitz{
			x = 9.3 * 3200,
			y = 15.5 * 3200,
			width = 8.4 * 3200,
			height = 1.3 * 3200,
			layer = 1600,
			colour = {r = 150, g = 90, b = 45},
		},
		bts.basicTextGlitz{
			x = 9.55 * 3200,
			y = 15.65 * 3200,
			text = "Boxes cannot be carried through",
			size = 1600,
			layer = 1600,
			colour = {r = 0, g = 0, b = 0},
		},
		bts.basicTextGlitz{
			x = 9.55 * 3200,
			y = (15.65 + 0.48) * 3200,
			text = "small portals - they don't fit.",
			size = 1600,
			layer = 1600,
			colour = {r = 0, g = 0, b = 0},
		},
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
