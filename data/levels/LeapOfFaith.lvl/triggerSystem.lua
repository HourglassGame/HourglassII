local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 28.5 * 3200, yOffset = 11.5 * 3200},
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
            width = 1 * 3200,
            height = 3 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 1,
            buttonTriggerID = 5,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 8.5 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    },
                    yDestination = {
                        desiredPosition = 3 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 8.5 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    },
                    yDestination = {
                        desiredPosition = 6 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    }
                }
            }
        },
        {
            width = 1 * 3200,
            height = 3 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 2,
            buttonTriggerID = 5,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 22.5 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    },
                    yDestination = {
                        desiredPosition = 3 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 22.5 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    },
                    yDestination = {
                        desiredPosition = 6 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    }
                }
            }
        },
        {
            width = 2 * 3200,
            height = 1 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 3,
            buttonTriggerID = 5,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 15 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    },
                    yDestination = {
                        desiredPosition = 3 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 15 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    },
                    yDestination = {
                        desiredPosition = 6 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    }
                }
            }
        },
        {
            width = 3 * 3200,
            height = 1 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 4,
            buttonTriggerID = 6,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 25 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    },
                    yDestination = {
                        desiredPosition = 9.5 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 28 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    },
                    yDestination = {
                        desiredPosition = 9.5 * 3200,
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
            attachment = {platform = nil, xOffset = 10.35 * 3200, yOffset = 8.1 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 7,
            pickupType = 'timeGun'
        },
        bts.pickup{
            attachment = {platform = nil, xOffset = 6 * 3200, yOffset = 8.1 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 8,
            pickupType = 'timeGun'
        },
        bts.pickup{
            attachment = {platform = nil, xOffset = 7.25 * 3200, yOffset = 8.1 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 9,
            pickupType = 'timeGun'
        },
        bts.spikes{
            attachment = {platform = nil, xOffset = 9 * 3200, yOffset = 17.5 * 3200},
            width = 14 * 3200,
            height = 0.5 * 3200,
            timeDirection = 'forwards',
        },
    },
    protoButtons = {
        bts.momentarySwitch{
            triggerID = 5,
            timeDirection = 'forwards',
            attachment = {platform = nil, xOffset = 4.5 * 3200, yOffset = 8.75 * 3200 },
            width = 3200,
            height = 800,
        },
        bts.momentarySwitch{
            triggerID = 6,
            timeDirection = 'forwards',
            attachment = {platform = nil, xOffset = 26.5 * 3200, yOffset = 8.75 * 3200 },
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
