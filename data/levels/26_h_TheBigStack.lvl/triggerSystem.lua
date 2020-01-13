local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 8.5 * 3200, yOffset = 10 * 3200},
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
            winner = false
        },
        {
            attachment = {platform = nil, xOffset = 26.5 * 3200, yOffset = 4 * 3200},
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
            illegalDestination = 2,
            fallable = false,
            winner = true
        }
    },
    protoCollisions = {
    },
    protoMutators = {
        bts.pickup{
            attachment = {platform = nil, xOffset = 13.75 * 3200, yOffset = 14.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 1,
            pickupType = 'timeGun'
        },
        bts.pickup{
            attachment = {platform = nil, xOffset = 16.75 * 3200, yOffset = 14.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 2,
            pickupType = 'timeGun'
        },
    },
    protoButtons = {
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
