local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 2.5 * 3200, yOffset = 7 * 3200},
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
            attachment = {platform = nil, xOffset = 28.5 * 3200, yOffset = 13 * 3200},
            index = 2,
            width = 2 * 3200,
            height = 2 * 3200,
            chargeTriggerID = 6,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = 2,
            xDestination = 0,
            yDestination = 0,
            relativeTime = true,
            timeDestination = 60 * 60,
            illegalDestination = 2,
            fallable = false,
            winner = false
        },
    },
    protoCollisions = {
        {
            width = 1 * 3200,
            height = 2 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 2,
            buttonTriggerID = 1,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 5.5 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    },
                    yDestination = {
                        desiredPosition = 5 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 5.5 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    },
                    yDestination = {
                        desiredPosition = 7 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    }
                }
            }
        },
        {
            width = 1 * 3200,
            height = 1 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 7,
            buttonTriggerID = 5,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 20 * 3200,
                        maxSpeed = 20,
                        acceleration = 20,
                        deceleration = 20
                    },
                    yDestination = {
                        desiredPosition = 16 * 3200,
                        maxSpeed = 20,
                        acceleration = 20,
                        deceleration = 20
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 20 * 3200,
                        maxSpeed = 20,
                        acceleration = 20,
                        deceleration = 20
                    },
                    yDestination = {
                        desiredPosition = 15 * 3200,
                        maxSpeed = 20,
                        acceleration = 20,
                        deceleration = 20
                    }
                }
            }
        },
    },
    protoMutators = {
        bts.pickup{
            attachment = {platform = nil, xOffset = 10.25 * 3200, yOffset = 8.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 3,
            pickupType = 'timeReverse'
        },
        bts.pickup{
            attachment = {platform = nil, xOffset = 27.75 * 3200, yOffset = 14.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 4,
            pickupType = 'timeGun'
        },
        bts.spikes{
            attachment = {platform = nil, xOffset = 11 * 3200, yOffset = 17.5 * 3200},
            width = 6 * 3200,
            height = 0.5 * 3200,
            timeDirection = 'forwards',
        },
        bts.spikes{
            attachment = {platform = 2, xOffset = -1 * 3200, yOffset = -1.5 * 3200},
            width = 7 * 3200,
            height = 0.5 * 3200,
            timeDirection = 'forwards',
        },
    },
    protoButtons = {
        bts.momentarySwitch{
            triggerID = 1,
            timeDirection = 'forwards',
            attachment = {platform = nil, xOffset = 26.5 * 3200, yOffset = 14.75 * 3200 },
            width = 3200,
            height = 800,
        },
        bts.stickySwitch{
            attachment = {platform = nil, xOffset = 18.75 * 3200, yOffset = 15.25 * 3200},
            width = 800,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 5
        }
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
