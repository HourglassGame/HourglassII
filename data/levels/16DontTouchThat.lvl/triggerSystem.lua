local bts = require "global.basicTriggerSystem"

local tempStore =
{
    --constant proto-object data:
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 9 * 3200, yOffset = 16 * 3200},
            index = 1,
            width = 2 * 3200,
            height = 2 * 3200,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = 1,
            xDestination = 0,
            yDestination = 0,
            relativeTime = true,
            timeDestination = -10 * 60,
            illegalDestination = 1,
            fallable = false,
            winner = false
        },
        {
            attachment = {platform = nil, xOffset = 18.5 * 3200, yOffset = 6.5 * 3200},
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
        }
    },
    protoCollisions = {
    },
    protoMutators = {
        bts.pickup{
            attachment = {platform = nil, xOffset = 11.75 * 3200, yOffset = 17.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 1,
            pickupType = 'timeJump'
        },
        bts.spikes{
            attachment = {platform = nil, xOffset = 8 * 3200, yOffset = 1 * 3200},
            width = 0.5 * 3200,
            height = 13 * 3200,
            timeDirection = 'forwards',
        },
        bts.spikes{
            attachment = {platform = nil, xOffset = 20 * 3200, yOffset = 14 * 3200},
            width = 0.5 * 3200,
            height = 4 * 3200,
            timeDirection = 'forwards',
        },
        bts.spikes{
            attachment = {platform = nil, xOffset = 20 * 3200, yOffset = 1 * 3200},
            width = 10 * 3200,
            height = 0.5 * 3200,
            timeDirection = 'forwards',
        },
        bts.spikes{
            attachment = {platform = nil, xOffset = 13.5 * 3200, yOffset = 17.5 * 3200},
            width = 1.5 * 3200,
            height = 0.5 * 3200,
            timeDirection = 'forwards',
        },
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
function mutateObject(responsibleManipulatorIndices, dynamicObject)
    return dynamicObject
end

getDepartureInformation = bts.getDepartureInformation(tempStore)

