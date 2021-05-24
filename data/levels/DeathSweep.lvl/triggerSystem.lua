local bts = require "global.basicTriggerSystem"

local tempStore =
{
	protoPortals =
	{
		{
			attachment = {platform = nil, xOffset = 25.5 * 3200, yOffset = 4 * 3200},
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
			attachment = {platform = nil, xOffset = 3.5 * 3200, yOffset = 4 * 3200},
			index = 2,
			width = 2 * 3200,
			height = 2 * 3200,
			collisionOverlap = 50,
			timeDirection = 'forwards',
			destinationDirection = 'reverse',
			relativeDirection = true,
			destinationIndex = 2,
			xDestination = 0,
			yDestination = 0,
			relativeTime = false,
			timeDestination = 10800 / 2,
			illegalDestination = 2,
			fallable = false,
			winner = false
		},
	},
	protoCollisions = {
		{
			width = 0 * 3200,
			height = 0 * 3200,
			timeDirection = 'forwards',
			lastStateTriggerID = 5,
			triggerClause = "t2 0 > t3 0 > t4 0 > + + 2 %",
			destinations =
			{
				onDestination = {
					xDestination = {
						desiredPosition = 5.5 * 3200,
						maxSpeed = 500,
						acceleration = 120,
						deceleration = 80
					},
					yDestination = {
						desiredPosition = 0 * 3200,
						maxSpeed = 260,
						acceleration = 100,
						deceleration = 150
					}
				},
				offDestination = {
					xDestination = {
						desiredPosition = 0 * 3200,
						maxSpeed = 500,
						acceleration = 120,
						deceleration = 80
					},
					yDestination = {
						desiredPosition = 0 * 3200,
						maxSpeed = 260,
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
			lastStateTriggerID = 6,
			buttonTriggerID = 1,
			destinations =
			{
				onDestination = {
					xDestination = {
						desiredPosition = 19 * 3200,
						maxSpeed = 300,
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
						desiredPosition = 19 * 3200,
						maxSpeed = 300,
						acceleration = 50,
						deceleration = 50
					},
					yDestination = {
						desiredPosition = 5 * 3200,
						maxSpeed = 300,
						acceleration = 50,
						deceleration = 50
					}
				}
			}
		},
		{
			width = 2 * 3200,
			height = 1 * 3200,
			timeDirection = 'forwards',
			lastStateTriggerID = 7,
			buttonTriggerID = 1,
			destinations =
			{
				onDestination = {
					xDestination = {
						desiredPosition = 19 * 3200,
						maxSpeed = 300,
						acceleration = 50,
						deceleration = 50
					},
					yDestination = {
						desiredPosition = 6 * 3200,
						maxSpeed = 300,
						acceleration = 50,
						deceleration = 50
					}
				},
				offDestination = {
					xDestination = {
						desiredPosition = 21 * 3200,
						maxSpeed = 300,
						acceleration = 50,
						deceleration = 50
					},
					yDestination = {
						desiredPosition = 6 * 3200,
						maxSpeed = 300,
						acceleration = 50,
						deceleration = 50
					}
				}
			}
		},
		{
			width = 1 * 3200,
			height = 2 * 3200,
			timeDirection = 'forwards',
			lastStateTriggerID = 8,
			buttonTriggerID = 1,
			destinations =
			{
				onDestination = {
					xDestination = {
						desiredPosition = 24 * 3200,
						maxSpeed = 300,
						acceleration = 50,
						deceleration = 50
					},
					yDestination = {
						desiredPosition = 4 * 3200,
						maxSpeed = 300,
						acceleration = 50,
						deceleration = 50
					}
				},
				offDestination = {
					xDestination = {
						desiredPosition = 24 * 3200,
						maxSpeed = 300,
						acceleration = 50,
						deceleration = 50
					},
					yDestination = {
						desiredPosition = 2 * 3200,
						maxSpeed = 300,
						acceleration = 50,
						deceleration = 50
					}
				}
			}
		},
	},
	protoMutators = {
		bts.spikes{
			attachment = {platform = 1, xOffset = 10 * 3200, yOffset = 12 * 3200},
			width = 0.5 * 3200,
			height = 5 * 3200,
			timeDirection = 'forwards',
		},
	},
	protoButtons = {
		bts.momentarySwitch{
			triggerID = 1,
			timeDirection = 'forwards',
			attachment = {platform = nil, xOffset = 12.5 * 3200, yOffset = 16.75 * 3200},
			width = 3200,
			height = 800,
		},
		bts.stickyLaserSwitch{
			triggerID = 2,
			timeDirection = 'forwards',
			attachment = {platform = nil, xOffset = 19.5 * 3200, yOffset = 9 * 3200 },
			beamLength = 2*3200,
			beamDirection = 3,
		},
		bts.stickyLaserSwitch{
			triggerID = 3,
			timeDirection = 'forwards',
			attachment = {platform = nil, xOffset = 23 * 3200, yOffset = 6.5 * 3200 },
			beamLength = 2*3200,
			beamDirection = 2,
		},
		bts.stickyLaserSwitch{
			triggerID = 4,
			timeDirection = 'forwards',
			attachment = {platform = nil, xOffset = 24.5 * 3200, yOffset = 6 * 3200 },
			beamLength = 2*3200,
			beamDirection = 3,
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
