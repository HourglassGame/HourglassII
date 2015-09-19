local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
		{
            attachment = {platform = nil, xOffset = 9 * 3200, yOffset = 6 * 3200},
            index = 1,
            width = 2 * 3200,
            height = 2 * 3200,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = 1,
            xDestination = 0,
            yDestination = 0,
            relativeTime = false,
            timeDestination = 4199,
            illegalDestination = 1,
            fallable = false,
            winner = false
        },
        {
            attachment = {platform = nil, xOffset = 11.5 * 3200, yOffset = 6 * 3200},
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
            illegalDestination = 1,
            fallable = false,
            winner = true
        },
    },
    protoCollisions = {
    },
	protoMutators = {
        bts.pickup{
            attachment = {platform = nil, xOffset = 4.75 * 3200, yOffset = 12.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 1,
			pickupType = 'timeReverse'
        },
		bts.spikes{
			attachment = {platform = nil, xOffset = 1 * 3200, yOffset = 12.5 * 3200},
            width = 0.5 * 3200,
            height = 1.5 * 3200,
            timeDirection = 'forwards',
		},
		bts.spikes{
			attachment = {platform = nil, xOffset = 1 * 3200, yOffset = 13.5 * 3200},
            width = 1.5 * 3200,
            height = 0.5 * 3200,
            timeDirection = 'forwards',
		},
		bts.spikes{
			attachment = {platform = nil, xOffset = 12.5 * 3200, yOffset = 1 * 3200},
            width = 1.5 * 3200,
            height = 0.5 * 3200,
            timeDirection = 'forwards',
		},
		bts.spikes{
			attachment = {platform = nil, xOffset = 13.5 * 3200, yOffset = 1 * 3200},
            width = 0.5 * 3200,
            height = 1.5 * 3200,
            timeDirection = 'forwards',
		},
	},
    protoButtons = {
    },
	protoGlitz = {
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
