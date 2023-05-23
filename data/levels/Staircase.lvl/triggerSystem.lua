local bts = require "global.basicTriggerSystem"

local tempStore =
{
	--constant proto-object data:
	protoPortals =
	{
		{
			attachment = {platform = nil, xOffset = 25 * 3200, yOffset = 7 * 3200},
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
			attachment = {platform = nil, xOffset = 15.5 * 3200, yOffset = 13 * 3200},
			index = 2,
			width = 2 * 3200,
			height = 2 * 3200,
			collisionOverlap = 50,
			timeDirection = 'forwards',
			destinationIndex = 2,
			xDestination = 0,
			yDestination = 0,
			relativeTime = true,
			destinationDirection = 'reverse',
			timeDestination = 0 * 60,
			illegalDestination = 2,
			chargeTriggerID = 1,
			fallable = false,
			winner = false,
		},
		{
			attachment = {platform = nil, xOffset = 5 * 3200, yOffset = 7 * 3200},
			index = 3,
			width = 2 * 3200,
			height = 2 * 3200,
			collisionOverlap = 50,
			timeDirection = 'forwards',
			destinationIndex = 3,
			xDestination = 0,
			yDestination = 0,
			relativeTime = false,
			timeDestination = 0 * 60,
			illegalDestination = 3,
			fallable = false,
			winner = false,
		},
	},
	protoCollisions = {
	},
	protoMutators = {
	},
	protoButtons = {
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
