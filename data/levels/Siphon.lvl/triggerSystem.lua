local bts = require "global.basicTriggerSystem"

local tempStore =
{
	protoPortals =
	{
		{
			attachment = {platform = nil, xOffset = 11.5 * 3200, yOffset = 2 * 3200},
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
			attachment = {platform = nil, xOffset = 26.5 * 3200, yOffset = 7 * 3200},
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
	},
	protoMutators = {
		bts.pickup{
			attachment = {platform = nil, xOffset = 19.25 * 3200, yOffset = 7 * 3200},
			width = 1600,
			height = 1600,
			timeDirection = 'forwards',
			triggerID = 2,
			pickupType = 'timeJump'
		},
		bts.elevator{
			attachment = {platform = nil, xOffset = 11 * 3200, yOffset = 9 * 3200},
			width = 2 * 3200,
			height = 7 * 3200,
			timeDirection = 'forwards',
			triggerID = 3,
			buttonTriggerID = 1,
			acceleration = 35,
			maxSpeed = 400,
			requireJump = false,
		},
	},
	protoButtons = {
		bts.stickyLaserSwitch{
			triggerID = 1,
			timeDirection = 'forwards',
			attachment = {platform = nil, xOffset = 25 * 3200, yOffset = 9 * 3200 },
			beamLength = 2 * 3200,
			beamDirection = 3,
		},
	},
	protoGlitz = {
		bts.easyWireGlitz{
			x = {pos = 25 * 3200},
			y1 = {pos = 9 * 3200},
			y2 = {pos = 14.55 * 3200},
			triggerID = 1,
			useTriggerArrival = true
		},
		bts.easyWireGlitz{
			x1 = {pos = 13 * 3200},
			x2 = {pos = 25 * 3200},
			y = {pos = 14.5 * 3200},
			triggerID = 1,
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
