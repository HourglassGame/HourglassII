local bts = require "global.basicTriggerSystem"

local tempStore = 
{
	protoPortals =
	{
		{
			attachment = {platform = nil, xOffset = 18.5 * 3200, yOffset = 3 * 3200},
			index = 1,
			width = 2 * 3200,
			height = 2 * 3200,
			collisionOverlap = 50,
			timeDirection = 'forwards',
			destinationIndex = 1,
			xDestination = 0,
			yDestination = 0,
			relativeTime = true,
			timeDestination = -8 * 60,
			illegalDestination = 1,
			fallable = false,
			winner = true
		}
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
						desiredPosition = 8 * 3200,
						maxSpeed = 250,
						acceleration = 20,
						deceleration = 20
					},
					yDestination = {
						desiredPosition = 5 * 3200,
						maxSpeed = 300,
						acceleration = 30,
						deceleration = 20
					}
				},
				offDestination = {
					xDestination = {
						desiredPosition = 8 * 3200,
						maxSpeed = 250,
						acceleration = 20,
						deceleration = 20
					},
					yDestination = {
						desiredPosition = 13 * 3200,
						maxSpeed = 250,
						acceleration = 20,
						deceleration = 20
					}
				}
			}
		},
		{
			width = 2 * 3200,
			height = 1 * 3200,
			timeDirection = 'forwards',
			lastStateTriggerID = 4,
			buttonTriggerID = 1,
			destinations =
			{
				onDestination = {
					xDestination = {
						desiredPosition = 22 * 3200,
						maxSpeed = 250,
						acceleration = 20,
						deceleration = 20
					},
					yDestination = {
						desiredPosition = 5 * 3200,
						maxSpeed = 300,
						acceleration = 30,
						deceleration = 20
					}
				},
				offDestination = {
					xDestination = {
						desiredPosition = 22 * 3200,
						maxSpeed = 250,
						acceleration = 20,
						deceleration = 20
					},
					yDestination = {
						desiredPosition = 13 * 3200,
						maxSpeed = 250,
						acceleration = 20,
						deceleration = 20
					}
				}
			}
		},
	},
	protoMutators = {
		bts.pickup{
			attachment = {platform = nil, xOffset = 7.75 * 3200, yOffset = 12.25 * 3200},
			width = 1600,
			height = 1600,
			timeDirection = 'forwards',
			triggerID = 5,
			pickupType = 'timeGun'
		},
		bts.pickup{
			attachment = {platform = nil, xOffset = 23.75 * 3200, yOffset = 12.25 * 3200},
			width = 1600,
			height = 1600,
			timeDirection = 'forwards',
			triggerID = 6,
			pickupType = 'timeJump'
		},
	},
	protoButtons = {
		bts.multiStickySwitch{
			buttons = {
				{
					attachment = {platform = nil, xOffset = 3 * 3200, yOffset = 11.25 * 3200},
					width = 800,
					height = 1600,
				},
				{
					attachment = {platform = nil, xOffset = 28.75 * 3200, yOffset = 11.25 * 3200},
					width = 800,
					height = 1600,
				},
			},
			timeDirection = 'forwards',
			triggerID = 1,
			stateTriggerID = 2
		},
	},
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
