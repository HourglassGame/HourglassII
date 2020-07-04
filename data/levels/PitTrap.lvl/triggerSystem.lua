local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 26.5 * 3200, yOffset = 2 * 3200},
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
        {
            width = 2 * 3200,
            height = 0.5 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 1,
            buttonTriggerID = 3,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 23 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    },
                    yDestination = {
                        desiredPosition = 4 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 23 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    },
                    yDestination = {
                        desiredPosition = 8.5 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    }
                }
            }
        },
        {
            width = 8 * 3200,
            height = 1 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 2,
            triggerClause = "0",
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 12 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    },
                    yDestination = {
                        desiredPosition = 14 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 12 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    },
                    yDestination = {
                        desiredPosition = 14 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    }
                }
            }
        },
    },
    protoMutators = {
        bts.pickup{
            attachment = {platform = nil, xOffset = 9.25 * 3200, yOffset = 8.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 4,
            pickupType = 'timeReverse'
        },
        bts.pickup{
            attachment = {platform = nil, xOffset = 21.25 * 3200, yOffset = 13.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 5,
            pickupType = 'timeReverse'
        },
        bts.pickup{
            attachment = {platform = nil, xOffset = 10.25 * 3200, yOffset = 13.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 6,
            pickupType = 'timeJump'
        },
    },
    protoButtons = {
        bts.momentarySwitch{
            triggerID = 3,
            timeDirection = 'forwards',
            attachment = {platform = nil, xOffset = 15.5 * 3200, yOffset = 17.75 * 3200 },
            width = 3200,
            height = 800,
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
