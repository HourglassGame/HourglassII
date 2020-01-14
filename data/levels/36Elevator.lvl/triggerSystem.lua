local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 28.5 * 3200, yOffset = 11 * 3200},
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
        }
    },
    protoCollisions = {
        {
            width = 3 * 3200,
            height = 0.5 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 3,
            buttonTriggerID = 1,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 14 * 3200,
                        maxSpeed = 180,
                        acceleration = 16,
                        deceleration = 16
                    },
                    yDestination = {
                        desiredPosition = 16 * 3200,
                        maxSpeed = 180,
                        acceleration = 16,
                        deceleration = 16
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 14 * 3200,
                        maxSpeed = 180,
                        acceleration = 16,
                        deceleration = 16
                    },
                    yDestination = {
                        desiredPosition = 10 * 3200,
                        maxSpeed = 180,
                        acceleration = 16,
                        deceleration = 16
                    }
                }
            }
        },
        {
            width = 1 * 3200,
            height = 2 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 4,
            buttonTriggerID = 2,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 17 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    },
                    yDestination = {
                        desiredPosition = 12 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 17 * 3200,
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
            attachment = {platform = nil, xOffset = 17.75 * 3200, yOffset = 9.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 5,
            pickupType = 'timeJump'
        },
        bts.pickup{
            attachment = {platform = nil, xOffset = 18.75 * 3200, yOffset = 9.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 6,
            pickupType = 'timeJump'
        },
        bts.pickup{
            attachment = {platform = nil, xOffset = 19.75 * 3200, yOffset = 9.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 7,
            pickupType = 'timeJump'
        }
    },
    protoButtons = {
        bts.momentarySwitch{
            triggerID = 1,
            timeDirection = 'forwards',
            attachment = {platform = nil, xOffset = 11.5 * 3200, yOffset = 9.75 * 3200 },
            width = 3200,
            height = 800,
        },
        bts.stickySwitch{
            attachment = {platform = nil, xOffset = 3 * 3200, yOffset = 12.25 * 3200},
            width = 800,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 2
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
