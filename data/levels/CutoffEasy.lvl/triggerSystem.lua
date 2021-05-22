local bts = require "global.basicTriggerSystem"

local tempStore =
{
	protoPortals =
	{
		{
			attachment = {platform = nil, xOffset = 3.5 * 3200, yOffset = 7 * 3200},
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
		}
	},
	protoCollisions = {
		{
			width = 1 * 3200,
			height = 3.5 * 3200,
			timeDirection = 'forwards',
			lastStateTriggerID = 3,
			buttonTriggerID = 1,
			destinations =
			{
				onDestination = {
					xDestination = {
						desiredPosition = 20.5 * 3200,
						maxSpeed = 260,
						acceleration = 30,
						deceleration = 30
					},
					yDestination = {
						desiredPosition = 6 * 3200,
						maxSpeed = 300,
						acceleration = 30,
						deceleration = 30
					}
				},
				offDestination = {
					xDestination = {
						desiredPosition = 20.5 * 3200,
						maxSpeed = 260,
						acceleration = 30,
						deceleration = 30
					},
					yDestination = {
						desiredPosition = 9.5 * 3200,
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
			buttonTriggerID = 7,
			destinations =
			{
				onDestination = {
					xDestination = {
						desiredPosition = 6.5 * 3200,
						maxSpeed = 400,
						acceleration = 60,
						deceleration = 60
					},
					yDestination = {
						desiredPosition = 7 * 3200,
						maxSpeed = 400,
						acceleration = 60,
						deceleration = 60
					}
				},
				offDestination = {
					xDestination = {
						desiredPosition = 6.5 * 3200,
						maxSpeed = 400,
						acceleration = 60,
						deceleration = 60
					},
					yDestination = {
						desiredPosition = 5 * 3200,
						maxSpeed = 400,
						acceleration = 60,
						deceleration = 60
					}
				}
			}
		},
	},
	protoMutators = {
		bts.pickup{
			attachment = {platform = nil, xOffset = 11.25 * 3200, yOffset = 12 * 3200},
			width = 1600,
			height = 1600,
			timeDirection = 'forwards',
			triggerID = 5,
			pickupType = 'timeJump'
		},
		bts.pickup{
			attachment = {platform = nil, xOffset = 17.25 * 3200, yOffset = 8 * 3200},
			width = 1600,
			height = 1600,
			timeDirection = 'forwards',
			triggerID = 6,
			pickupType = 'timeJump'
		},
	},
	protoButtons = {
		bts.momentarySwitch{
			triggerID = 1,
			timeDirection = 'forwards',
			attachment = {platform = nil, xOffset = 24.5 * 3200, yOffset = 12.75 * 3200},
			width = 3200,
			height = 800,
		},
		bts.stickyLaserSwitch{
			triggerID = 2,
			timeDirection = 'forwards',
			attachment = {platform = nil, xOffset = 19.25 * 3200, yOffset = 6 * 3200 },
			beamLength = 3*3200,
			beamDirection = 1,
		},
	},
	protoTriggerMods = {
		{
			triggerID = 7,
			triggerClause = "t2 t7 |",
		}
	},
	protoGlitz = {
		bts.easyWireGlitz{
			x = {pos = 25 * 3200},
			y1 = {pos = 13 * 3200},
			y2 = {pos = 13.75 * 3200},
			triggerID = 1,
			useTriggerArrival = true
		},
		bts.easyWireGlitz{
			x1 = {pos = 20.95 * 3200},
			x2 = {pos = 25.05 * 3200},
			y = {pos = 13.75 * 3200},
			triggerID = 1,
			useTriggerArrival = true
		},
		bts.easyWireGlitz{
			x = {pos = 21 * 3200},
			y1 = {pos = 13 * 3200},
			y2 = {pos = 13.75 * 3200},
			triggerID = 1,
			useTriggerArrival = true
		},
		bts.easyWireGlitz{
			x = {pos = 19.25 * 3200},
			y1 = {pos = 6 * 3200},
			y2 = {pos = 4.25 * 3200},
			triggerID = 2,
			useTriggerArrival = true
		},
		bts.basicRectangleGlitz{
			x = 18.5 * 3200,
			y = 3.4 * 3200,
			width = 1.5 * 3200,
			height = 1.5 * 3200,
			layer = 1600,
			colour = {r = 120, g = 120, b = 120},
		},
		bts.basicTextGlitz{
			x = 18.6 * 3200,
			y = 3.85 * 3200,
			text = "Reverse",
			size = 1200,
			layer = 1600,
			colour = {r = 255, g = 255, b = 255},
		},
		bts.easyWireGlitz{
			x1 = {pos = 19.25 * 3200},
			x2 = {pos = 6.95 * 3200},
			y = {pos = 4.15 * 3200},
			triggerID = 7,
			useTriggerArrival = true
		},
		bts.easyWireGlitz{
			x = {pos = 7 * 3200},
			y1 = {pos = 4.15 * 3200},
			y2 = {pos = 7 * 3200},
			triggerID = 7,
			useTriggerArrival = true
		},
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
