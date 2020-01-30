local bts = require "global.basicTriggerSystem"

local tempStore =
{
    --constant proto-object data:
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 15 * 3200, yOffset = 11.2 * 3200},
            index = 1,
            width = 2 * 3200,
            height = 2 * 3200,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = 1,
            xDestination = 0,
            yDestination = 0,
            relativeTime = false,
            timeDestination = 0 * 60,
            illegalDestination = 1,
            fallable = false,
            winner = false,
            chargeTriggerID = 10,
        },
        {
            attachment = {platform = nil, xOffset =  23 * 3200, yOffset = 5 * 3200},
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
            winner = true
        }
    },
    protoCollisions = {
        {
            width = 2 * 3200,
            height = 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 3,
            buttonTriggerID = 1,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 10 * 3200,
                        maxSpeed = 200,
                        acceleration = 15,
                        deceleration = 8
                    },
                    yDestination = {
                        desiredPosition = 7 * 3200,
                        maxSpeed = 200,
                        acceleration = 15,
                        deceleration = 8
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 10 * 3200,
                        maxSpeed = 200,
                        acceleration = 15,
                        deceleration = 8
                    },
                    yDestination = {
                        desiredPosition = 16 * 3200,
                        maxSpeed = 200,
                        acceleration = 15,
                        deceleration = 8
                    }
                }
            }
        },
        {
            width = 2 * 3200,
            height = 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 4,
            buttonTriggerID = 2,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 20 * 3200,
                        maxSpeed = 200,
                        acceleration = 15,
                        deceleration = 8
                    },
                    yDestination = {
                        desiredPosition = 7 * 3200,
                        maxSpeed = 200,
                        acceleration = 15,
                        deceleration = 8
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 20 * 3200,
                        maxSpeed = 200,
                        acceleration = 15,
                        deceleration = 8
                    },
                    yDestination = {
                        desiredPosition = 16 * 3200,
                        maxSpeed = 200,
                        acceleration = 15,
                        deceleration = 8
                    }
                }
            }
        },
    },
    protoMutators = {
        bts.pickup{
            attachment = {platform = nil, xOffset = 8.75 * 3200, yOffset = 4.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 8,
            pickupType = 'timeReverse'
        },
        bts.pickup{
            attachment = {platform = nil, xOffset = 20.75 * 3200, yOffset = 15.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 9,
            pickupType = 'timeReverse'
        },
        bts.spikes{
            attachment = {platform = nil, xOffset = 14 * 3200, yOffset = 7.75 * 3200},
            width = 4 * 3200,
            height = 0.5 * 3200,
            timeDirection = 'forwards',
        },
    },
    protoButtons = {
        bts.stickySwitch{
            attachment = {platform = nil, xOffset = 13.25 * 3200, yOffset = 17 * 3200 - 800},
            width = 1600,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 1
        },
        bts.stickySwitch{
            attachment = {platform = nil, xOffset = 14.25 * 3200, yOffset = 3 * 3200},
            width = 1600,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 2
        },
        bts.momentarySwitch{
            attachment = {platform = nil, xOffset = 18 * 3200, yOffset = 17 * 3200 - 800},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 6
        },
    },
    protoTriggerMods = {
        {
            triggerID = 5,
            triggerClause = "o6 1 & 24 |",
        },
        {
            triggerID = 7,
            triggerClause = "o6 10 & 100 |",
        }
    },
    protoGlitz = {
        bts.wireGlitz{
            x1 = {platform = nil, pos = 17.3 * 3200},
            y1 = {platform = nil, pos = 17.3 * 3200 },
            x2 = {platform = nil, pos = (17.3 + 2.6 + 0.2) * 3200},
            y2 = {platform = nil, pos = (17.3 + 1.2 + 0.2) * 3200},
            triggerID = 6,
            useTriggerArrival = false
        },
        bts.basicRectangleGlitz{
            x = 17.4 * 3200,
            y = 17.4 * 3200,
            width = 2.6 * 3200,
            height = 1.2 * 3200,
            layer = 1600,
            colour = {r = 120, g = 120, b = 120},
        },
        bts.basicTextGlitz{
            x = 17.6 * 3200,
            y = 17.65 * 3200,
            text = "Slow Time",
            size = 1600,
            layer = 1600,
            colour = {r = 255, g = 255, b = 255},
        },
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

