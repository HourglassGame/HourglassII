local bts = require "global.basicTriggerSystem"

local tempStore =
{
	protoPortals =
	{
		{
			attachment = {platform = nil, xOffset = 25 * 3200, yOffset = 10 * 3200},
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
			attachment = {platform = nil, xOffset = 3.5 * 3200, yOffset = 11 * 3200},
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
			illegalDestination = 2,
			fallable = false,
			winner = false
		},
	},
	protoCollisions = {
		{
			width = 5 * 3200,
			height = 4.25 * 3200,
			timeDirection = 'forwards',
			lastStateTriggerID = 2,
			buttonTriggerID = 1,
			destinations =
			{
				onDestination = {
					xDestination = {
						desiredPosition = 15 * 3200,
						maxSpeed = 200,
						acceleration = 50,
						deceleration = 50
					},
					yDestination = {
						desiredPosition = 10.75 * 3200,
						maxSpeed = 200,
						acceleration = 8,
						deceleration = 40
					}
				},
				offDestination = {
					xDestination = {
						desiredPosition = 15 * 3200,
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
				}
			}
		}
	},
	protoMutators = {
		bts.pickup{
			attachment = {platform = 1, xOffset = 2.25 * 3200, yOffset = -0.75 * 3200},
			width = 1600,
			height = 1600,
			timeDirection = 'forwards',
			triggerID = 3,
			pickupType = 'timeJump'
		},
		bts.spikes{
			attachment = {platform = 1, xOffset = 0 * 3200, yOffset = 4.25 * 3200},
			width = 5 * 3200,
			height = 0.5 * 3200,
			timeDirection = 'forwards',
		},
	},
	protoButtons = {
		bts.stickySwitch{
			attachment = {platform = nil, xOffset = 17.25 * 3200, yOffset = 15.75 * 3200},
			width = 1600,
			height = 800,
			timeDirection = 'forwards',
			triggerID = 1
		}
	},
	protoGlitz = {
		bts.basicRectangleGlitz{
			x = 4.5 * 3200,
			y = 5 * 3200,
			width = 8.4 * 3200,
			height = 2.85 * 3200,
			layer = 1600,
			colour = {r = 150, g = 90, b = 45},
		},
		bts.basicTextGlitz{
			x = 4.75 * 3200,
			y = 5.15 * 3200,
			text = "Items can be used even if your",
			size = 1600,
			layer = 1600,
			colour = {r = 0, g = 0, b = 0},
		},
		bts.basicTextGlitz{
			x = 4.75 * 3200,
			y = (5.15 + 0.48) * 3200,
			text = "inventory is empty, the attempt",
			size = 1600,
			layer = 1600,
			colour = {r = 0, g = 0, b = 0},
		},
		bts.basicTextGlitz{
			x = 4.75 * 3200,
			y = (5.15 + 0.48*2) * 3200,
			text = "will just fail. This can be",
			size = 1600,
			layer = 1600,
			colour = {r = 0, g = 0, b = 0},
		},
		bts.basicTextGlitz{
			x = 4.75 * 3200,
			y = (5.15 + 0.48*3) * 3200,
			text = "useful if you later expect to",
			size = 1600,
			layer = 1600,
			colour = {r = 0, g = 0, b = 0},
		},
		bts.basicTextGlitz{
			x = 4.75 * 3200,
			y = (5.15 + 0.48*4) * 3200,
			text = "have had the item at the time.",
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
