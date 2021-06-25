local bts = require "global.basicTriggerSystem"

local tempStore =
{
	protoPortals =
	{
		{
			attachment = {platform = nil, xOffset = 5.5 * 3200, yOffset = 15 * 3200},
			index = 1,
			width = 2 * 3200,
			height = 2 * 3200,
			collisionOverlap = 50,
			timeDirection = 'forwards',
			destinationIndex = nil,
			xDestination = 0,
			yDestination = 0,
			relativeTime = false,
			timeDestination = 0,
			illegalDestination = nil,
			fallable = false,
			winner = true
		},
		{
			attachment = {platform = nil, xOffset = 27.5 * 3200, yOffset = 8 * 3200},
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
			width = 2 * 3200,
			height = 1 * 3200,
			timeDirection = 'forwards',
			lastStateTriggerID = 3,
			buttonTriggerID = 1,
			destinations =
			{
				onDestination = {
					xDestination = {
						desiredPosition = 17 * 3200,
						maxSpeed = 260,
						acceleration = 15,
						deceleration = 15
					},
					yDestination = {
						desiredPosition = 12 * 3200,
						maxSpeed = 300,
						acceleration = 30,
						deceleration = 15
					}
				},
				offDestination = {
					xDestination = {
						desiredPosition = 17 * 3200,
						maxSpeed = 260,
						acceleration = 30,
						deceleration = 30
					},
					yDestination = {
						desiredPosition = 17 * 3200,
						maxSpeed = 300,
						acceleration = 30,
						deceleration = 30
					}
				}
			}
		},
		{
			width = 1 * 3200,
			height = 2 * 3200,
			timeDirection = 'forwards',
			lastStateTriggerID = 4,
			triggerClause = "t2 t6 &",
			destinations =
			{
				onDestination = {
					xDestination = {
						desiredPosition = 10.5 * 3200,
						maxSpeed = 260,
						acceleration = 30,
						deceleration = 30
					},
					yDestination = {
						desiredPosition = 13 * 3200,
						maxSpeed = 260,
						acceleration = 30,
						deceleration = 30
					}
				},
				offDestination = {
					xDestination = {
						desiredPosition = 10.5 * 3200,
						maxSpeed = 260,
						acceleration = 30,
						deceleration = 30
					},
					yDestination = {
						desiredPosition = 15 * 3200,
						maxSpeed = 260,
						acceleration = 30,
						deceleration = 30
					}
				}
			}
		},
	},
	protoMutators = {
		bts.pickup{
			attachment = {platform = nil, xOffset = 10.5 * 3200, yOffset = 10 * 3200},
			width = 1600,
			height = 1600,
			timeDirection = 'forwards',
			triggerID = 5,
			pickupType = 'timeJump'
		},
		bts.spikes{
			attachment = {platform = nil, xOffset = 23 * 3200, yOffset = 8 * 3200},
			width = 4 * 3200,
			height = 0.5 * 3200,
			timeDirection = 'forwards',
		},
	},
	protoButtons = {
		bts.momentarySwitch{
			triggerID = 1,
			timeDirection = 'forwards',
			attachment = {platform = nil, xOffset = 10.25 * 3200, yOffset = 11.75 * 3200},
			width = 3200,
			height = 800,
		},
		bts.momentarySwitch{
			triggerID = 2,
			timeDirection = 'forwards',
			attachment = {platform = nil, xOffset = 2.75 * 3200, yOffset = 6.75 * 3200 },
			width = 3200,
			height = 800,
		},
		bts.momentarySwitch{
			triggerID = 6,
			timeDirection = 'forwards',
			attachment = {platform = nil, xOffset = 5.25 * 3200, yOffset = 6.75 * 3200 },
			width = 3200,
			height = 800,
		},
	},
	protoTriggerMods = {
	},
	protoGlitz = {
	}
}
--==Callin Definitions==--
--triggerArrivals have already had default values inserted by C++
--for trigger indices that did not arrive by the time this is called
calculatePhysicsAffectingStuff = bts.calculatePhysicsAffectingStuff(tempStore)

--responsible*Index gives the position in the list of the thing that
--is responsible for the callin happening.
--By 'the list' I mean the list of objects returned from calculatePhysicsAffectingStuff
--corresponding to the callin type (portals/pickups/killers)
--in particular, this means that this does *not* correspond to the 'index' field
--of a portal (the 'index' field for identifying illegal portals, but not for this)
function shouldArrive(dynamicObject)
	return true
end
function shouldPort(responsiblePortalIndex, dynamicObject, porterActionedPortal)
	return true
end
mutateObject = bts.mutateObject(tempStore)

getDepartureInformation = bts.getDepartureInformation(tempStore)
