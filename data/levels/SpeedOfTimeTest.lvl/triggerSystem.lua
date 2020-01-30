local bts = require "global.basicTriggerSystem"

local tempStore =
{
    --constant proto-object data:
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 23 * 3200, yOffset = 7 * 3200},
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
            winner = false,
            triggerClause = "f 600 >",
        },
        {
            attachment = {platform = nil, xOffset = 26 * 3200, yOffset = 7 * 3200},
            index = 2,
            width = 2 * 3200,
            height = 2 * 3200,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = 2,
            xDestination = 0,
            yDestination = 0,
            relativeTime = false,
            timeDestination = 10 * 60,
            illegalDestination = 2,
            fallable = false,
            winner = false
        },
        {
            attachment = {platform = nil, xOffset = 3 * 1600, yOffset = 27 * 1600},
            index = 3,
            width = 2 * 3200,
            height = 2 * 3200,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = 3,
            xDestination = 0,
            yDestination = 0,
            relativeTime = false,
            timeDestination = 0,
            illegalDestination = 3,
            fallable = false,
            winner = true
        }
    },
    protoCollisions = {
        {
            width = 3200,
            height = 3 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 2,
            buttonTriggerID = 1,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 8 * 3200,
                        maxSpeed = 200,
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
                        desiredPosition = 8 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 10 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                }
            }
        }
    },
    protoMutators = {
    },
    protoButtons = {
        bts.momentarySwitch{
            attachment = {platform = nil, xOffset = 21.5 * 3200, yOffset = 9 * 3200 - 800},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 1
        },
        bts.momentarySwitch{
            attachment = {platform = nil, xOffset = 10 * 3200, yOffset = 13 * 3200 - 800},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 4
        },
    },
    protoTriggerMods = {
        {
            triggerID = 3,
            triggerClause = "o4 1 & 24 |",
        },
        {
            triggerID = 5,
            triggerClause = "o4 10 & 100 |",
        }
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

