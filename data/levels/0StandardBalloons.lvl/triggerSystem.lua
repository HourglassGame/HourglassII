local bts = require "global.basicTriggerSystem"

local tempStore =
{
	protoPortals =
	{
		{
			attachment = {platform = nil, xOffset = 15 * 3200, yOffset = 16 * 3200},
			index = 1,
			width = 2 * 3200,
			height = 2 * 3200,
			collisionOverlap = 50,
			timeDirection = 'forwards',
			destinationIndex = 1,
			triggerClause = "t1 t2 &",
			xDestination = 0,
			yDestination = 0,
			relativeTime = false,
			timeDestination = 0,
			illegalDestination = 1,
			fallable = false,
			winner = true
		},
	},
	protoCollisions = {
	},
	protoMutators = {
		bts.pickup{
			attachment = {platform = nil, xOffset = 15.75 * 3200, yOffset = 11 * 3200},
			width = 1600,
			height = 1600,
			timeDirection = 'forwards',
			triggerID = 3,
			pickupType = 'timeGun'
		},
		bts.pickup{
			attachment = {platform = nil, xOffset = 5.75 * 3200, yOffset = 17 * 3200},
			width = 1600,
			height = 1600,
			timeDirection = 'forwards',
			triggerID = 4,
			pickupType = 'timeGun'
		},
		bts.pickup{
			attachment = {platform = nil, xOffset = 25.75 * 3200, yOffset = 17 * 3200},
			width = 1600,
			height = 1600,
			timeDirection = 'forwards',
			triggerID = 5,
			pickupType = 'timeJump'
		},
		bts.spikes{
			attachment = {platform = nil, xOffset = 1 * 3200, yOffset = 18.5 * 3200},
			width = 30 * 3200,
			height = 0.5 * 3200,
			timeDirection = 'forwards',
		},
	},
	protoButtons = {
		bts.momentarySwitch{
			attachment = {platform = nil, xOffset = 26.5 * 3200, yOffset = 7.75 * 3200},
			width = 4800,
			height = 800,
			pressForceReq = 2,
			timeDirection = 'forwards',
			triggerID = 1
		},
		bts.momentarySwitch{
			attachment = {platform = nil, xOffset = 4 * 3200, yOffset = 7.75 * 3200},
			width = 4800,
			height = 800,
			pressForceReq = 2,
			timeDirection = 'forwards',
			triggerID = 2
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