local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 6 * 3200, yOffset = 8 * 3200},
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
            attachment = {platform = nil, xOffset = 24 * 3200, yOffset = 8 * 3200},
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
        {
            width = 3200,
            height = 3 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 1,
            triggerClause = "0",
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 11 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 13 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 11 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 13 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                }
            }
        },
        {
            width = 3200,
            height = 3 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 2,
            triggerClause = "0",
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 20 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 13 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 20 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 13 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                }
            }
        },
    },
    protoMutators = {
        bts.pickup{
            attachment = {platform = nil, xOffset = 15.75 * 3200, yOffset = 14.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 3,
            pickupType = 'timeJump'
        },
        bts.pickup{
            attachment = {platform = nil, xOffset = 20.75 * 3200, yOffset = 3.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 4,
            pickupType = 'timeGun'
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
mutateObject = bts.mutateObject(tempStore)

getDepartureInformation = bts.getDepartureInformation(tempStore)
