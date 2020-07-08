local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 5.5 * 3200, yOffset = 3 * 3200},
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
    },
    protoCollisions = {
    },
    protoMutators = {
        bts.pickup{
            attachment = {platform = nil, xOffset = 22.25 * 3200, yOffset = 16.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 1,
            pickupType = 'timeJump'
        },
        bts.pickup{
            attachment = {platform = nil, xOffset = 15.75 * 3200, yOffset = 12.75 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 2,
            pickupType = 'timeJump'
        },
    },
    protoButtons = {
    }
}

local triggerID = 3
for i = 1, 4 do
	for j = 1, 4 do
		tempStore.protoMutators[#tempStore.protoMutators + 1] = bts.pickup{
            attachment = {platform = nil, xOffset = (5.5 + i*0.5) * 3200, yOffset = (9.5 + j*0.5)  * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = triggerID,
            pickupType = 'timeGun'
        }
		triggerID = triggerID + 1
	end
end

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
