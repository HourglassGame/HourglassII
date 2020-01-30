local bts = require "global.basicTriggerSystem"

local tempStore =
{
    --constant proto-object data:
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 12.5 * 3200, yOffset = 12 * 3200},
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
            triggerClause = "t9",
        },
        {
            attachment = {platform = nil, xOffset = 17.5 * 3200, yOffset = 12 * 3200},
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
            winner = true,
            triggerClause = "t9",
        }
    },
    protoCollisions = {
        {
            width = 2 * 3200,
            height = 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 6,
            triggerClause = "t1 t2 &",
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 5 * 3200,
                        maxSpeed = 200,
                        acceleration = 15,
                        deceleration = 15
                    },
                    yDestination = {
                        desiredPosition = 9 * 3200,
                        maxSpeed = 250,
                        acceleration = 15,
                        deceleration = 15
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 5 * 3200,
                        maxSpeed = 200,
                        acceleration = 15,
                        deceleration = 15
                    },
                    yDestination = {
                        desiredPosition = 17 * 3200,
                        maxSpeed = 250,
                        acceleration = 15,
                        deceleration = 15
                    }
                }
            }
        },
        {
            width = 2 * 3200,
            height = 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 7,
            triggerClause = "t3 t4 &",
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 25 * 3200,
                        maxSpeed = 200,
                        acceleration = 15,
                        deceleration = 15
                    },
                    yDestination = {
                        desiredPosition = 9 * 3200,
                        maxSpeed = 250,
                        acceleration = 15,
                        deceleration = 15
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 25 * 3200,
                        maxSpeed = 200,
                        acceleration = 15,
                        deceleration = 15
                    },
                    yDestination = {
                        desiredPosition = 17 * 3200,
                        maxSpeed = 250,
                        acceleration = 15,
                        deceleration = 15
                    }
                }
            }
        },
    },
    protoMutators = {
    },
    protoButtons = {
        bts.momentarySwitch{
            attachment = {platform = nil, xOffset = 9 * 3200, yOffset = 9 * 3200 - 800},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 1
        },
        bts.momentarySwitch{
            attachment = {platform = nil, xOffset = 11.5 * 3200, yOffset = 9 * 3200 - 800},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 2
        },
        bts.momentarySwitch{
            attachment = {platform = nil, xOffset = 19.5 * 3200, yOffset = 9 * 3200 - 800},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 3
        },
        bts.momentarySwitch{
            attachment = {platform = nil, xOffset = 22 * 3200, yOffset = 9 * 3200 - 800},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 4
        },
        bts.momentarySwitch{
            attachment = {platform = nil, xOffset = 28 * 3200, yOffset = 9 * 3200 - 800},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 5
        },
        bts.momentarySwitch{
            attachment = {platform = nil, xOffset = 15.5 * 3200, yOffset = 9 * 3200 - 800},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 9
        },
    },
    protoTriggerMods = {
        {
            triggerID = 8,
            triggerClause = "o5 1 & 24 |",
        }
    },
    protoGlitz = {
        bts.easyWireGlitz{
            x = {pos = 9.5 * 3200},
            y1 = {pos = 9 * 3200},
            y2 = {pos = 10 * 3200},
            triggerID = 1,
            useTriggerArrival = true
        },
        bts.easyWireGlitz{
            x = {pos = 12 * 3200},
            y1 = {pos = 9 * 3200},
            y2 = {pos = 10 * 3200},
            triggerID = 2,
            useTriggerArrival = true
        },
        bts.easyWireGlitz{
            x1 = {pos = 12 * 3200},
            x2 = {pos = 9.5 * 3200},
            y = {pos = 10 * 3200},
            triggerID = 2,
            useTriggerArrival = true
        },
        bts.easyWireGlitz{
            x1 = {pos = 7 * 3200},
            x2 = {pos = 9.5 * 3200},
            y = {pos = 10 * 3200},
            triggerClause = "t1 t2 &",
            useTriggerArrival = true
        },
        bts.basicRectangleGlitz{
            x = 9.2 * 3200,
            y = 9.7 * 3200,
            width = 0.6 * 3200,
            height = 0.6 * 3200,
            layer = 1600,
            colour = {r = 120, g = 120, b = 120},
        },
        bts.basicTextGlitz{
            x = 9.35 * 3200,
            y = 9.7 * 3200,
            text = "&",
            size = 1600,
            layer = 1600,
            colour = {r = 255, g = 255, b = 255},
        },
        bts.easyWireGlitz{
            x = {pos = 20 * 3200},
            y1 = {pos = 9 * 3200},
            y2 = {pos = 10 * 3200},
            triggerID = 3,
            useTriggerArrival = true
        },
        bts.easyWireGlitz{
            x = {pos = 22.5 * 3200},
            y1 = {pos = 9 * 3200},
            y2 = {pos = 10 * 3200},
            triggerID = 4,
            useTriggerArrival = true
        },
        bts.easyWireGlitz{
            x1 = {pos = 22.5 * 3200},
            x2 = {pos = 20 * 3200},
            y = {pos = 10 * 3200},
            triggerID = 3,
            useTriggerArrival = true
        },
        bts.easyWireGlitz{
            x1 = {pos = 25 * 3200},
            x2 = {pos = 22.5 * 3200},
            y = {pos = 10 * 3200},
            triggerClause = "t3 t4 &",
            useTriggerArrival = true
        },
        bts.basicRectangleGlitz{
            x = 22.2 * 3200,
            y = 9.7 * 3200,
            width = 0.6 * 3200,
            height = 0.6 * 3200,
            layer = 1600,
            colour = {r = 120, g = 120, b = 120},
        },
        bts.basicTextGlitz{
            x = 22.35 * 3200,
            y = 9.7 * 3200,
            text = "&",
            size = 1600,
            layer = 1600,
            colour = {r = 255, g = 255, b = 255},
        },
        bts.wireGlitz{
            x1 = {platform = nil, pos = 27.3 * 3200},
            y1 = {platform = nil, pos = 9.3 * 3200 },
            x2 = {platform = nil, pos = (27.3 + 2.6 + 0.2) * 3200},
            y2 = {platform = nil, pos = (9.3 + 1.2 + 0.2) * 3200},
            triggerID = 5,
            useTriggerArrival = false
        },
        bts.basicRectangleGlitz{
            x = 27.4 * 3200,
            y = 9.4 * 3200,
            width = 2.6 * 3200,
            height = 1.2 * 3200,
            layer = 1600,
            colour = {r = 120, g = 120, b = 120},
        },
        bts.basicTextGlitz{
            x = 27.6 * 3200,
            y = 9.65 * 3200,
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

