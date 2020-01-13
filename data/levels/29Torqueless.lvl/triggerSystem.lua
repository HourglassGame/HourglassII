local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 5.5 * 3200, yOffset = 12 * 3200},
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
            attachment = {platform = nil, xOffset = 24.5 * 3200, yOffset = 12 * 3200},
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
        bts.spikes{
            attachment = {platform = nil, xOffset = 11 * 3200, yOffset = 16.5 * 3200},
            width = 10 * 3200,
            height = 0.5 * 3200,
            timeDirection = 'forwards',
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
