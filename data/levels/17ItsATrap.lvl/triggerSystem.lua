local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 5 * 3200, yOffset = 14.5 * 3200},
            index = 1,
            width = 2 * 3200,
            height = 2 * 3200,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = nil,
            xDestination = 0,
            yDestination = 0,
            relativeTime = false,
            timeDestination = 0,
            illegalDestination = nil,
            fallable = false,
            winner = true
        }
    },
    protoCollisions = {
        {
            width = 2 * 3200,
            height = 0.5 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 3,
            buttonTriggerID = 1,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 5 * 3200,
                        maxSpeed = 260,
                        acceleration = 30,
                        deceleration = 30
                    },
                    yDestination = {
                        desiredPosition = 17.5 * 3200,
                        maxSpeed = 260,
                        acceleration = 30,
                        deceleration = 30
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 5 * 3200,
                        maxSpeed = 260,
                        acceleration = 30,
                        deceleration = 30
                    },
                    yDestination = {
                        desiredPosition = 9 * 3200,
                        maxSpeed = 260,
                        acceleration = 30,
                        deceleration = 30
                    }
                }
            }
        },
        {
            width = 0 * 3200,
            height = 0 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 4,
            buttonTriggerID = 2,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 0 * 3200,
                        maxSpeed = 260,
                        acceleration = 30,
                        deceleration = 30
                    },
                    yDestination = {
                        desiredPosition = 0 * 3200,
                        maxSpeed = 120,
                        acceleration = 150,
                        deceleration = 150
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 0 * 3200,
                        maxSpeed = 260,
                        acceleration = 30,
                        deceleration = 30
                    },
                    yDestination = {
                        desiredPosition = -1 * 3200,
                        maxSpeed = 260,
                        acceleration = 30,
                        deceleration = 30
                    }
                }
            }
        },
        {
            width = 0 * 3200,
            height = 0 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 5,
            buttonTriggerID = 2,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 0 * 3200,
                        maxSpeed = 260,
                        acceleration = 30,
                        deceleration = 30
                    },
                    yDestination = {
                        desiredPosition = 0 * 3200,
                        maxSpeed = 120,
                        acceleration = 150,
                        deceleration = 150
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 0 * 3200,
                        maxSpeed = 260,
                        acceleration = 30,
                        deceleration = 30
                    },
                    yDestination = {
                        desiredPosition = 1 * 3200,
                        maxSpeed = 260,
                        acceleration = 30,
                        deceleration = 30
                    }
                }
            }
        },
    },
    protoMutators = {
        bts.pickup{
            attachment = {platform = nil, xOffset = 8.25 * 3200, yOffset = 8 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 6,
            pickupType = 'timeJump'
        },
        bts.pickup{
            attachment = {platform = nil, xOffset = 23.25 * 3200, yOffset = 8 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 7,
            pickupType = 'timeReverse'
        },
        bts.spikes{
            attachment = {platform = 2, xOffset = 12 * 3200, yOffset = 7 * 3200},
            width = 9.5 * 3200,
            height = 0.5 * 3200,
            timeDirection = 'forwards',
        },
        bts.spikes{
            attachment = {platform = 3, xOffset = 12 * 3200, yOffset = 8.5 * 3200},
            width = 9.5 * 3200,
            height = 0.5 * 3200,
            timeDirection = 'forwards',
        },
    },
    protoButtons = {
        bts.momentarySwitch{
            triggerID = 1,
            timeDirection = 'forwards',
            attachment = {platform = nil, xOffset = 24.75 * 3200, yOffset = 7.5 * 3200},
            width = 800,
            height = 3200,
        },
        bts.stickyLaserSwitch{
            triggerID = 2,
            timeDirection = 'forwards',
            attachment = {platform = nil, xOffset = 21.75 * 3200, yOffset = 9 * 3200 },
            beamLength = 2*3200,
            beamDirection = 3,
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
function mutateObject(responsibleManipulatorIndices, dynamicObject)
    return dynamicObject
end
mutateObject = bts.mutateObject(tempStore)

getDepartureInformation = bts.getDepartureInformation(tempStore)
