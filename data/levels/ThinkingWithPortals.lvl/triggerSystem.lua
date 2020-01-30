local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {xOffset = 26.5 * 3200, yOffset = 7 * 3200},
            index = 1,
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
        },
        {
            attachment = {xOffset = 15 * 3200, yOffset = 14.5 * 3200},
            index = 2,
            width = 6400,
            height = 6400,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = 3,
            xDestination = 0,
            yDestination = 0,
            relativeTime = true,
            timeDestination = -300,
            illegalDestination = 3,
            fallable = true,
            winner = false,
        },
        {
            attachment = {xOffset = 15 * 3200, yOffset = 4.5 * 3200},
            index = 3,
            width = 6400,
            height = 6400,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = 2,
            xDestination = 0,
            yDestination = 0,
            relativeTime = true,
            timeDestination = 300,
            illegalDestination = 2,
            fallable = true,
            winner = false,
        }
    },
    protoCollisions = {
        {
            width = 4 * 3200,
            height = 4 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 2,
            buttonTriggerID = 1,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 14 * 3200,
                        maxSpeed = 200,
                        acceleration = 16,
                        deceleration = 16
                    },
                    yDestination = {
                        desiredPosition = 16.5 * 3200,
                        maxSpeed = 300,
                        acceleration = 16,
                        deceleration = 16
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 14 * 3200,
                        maxSpeed = 200,
                        acceleration = 16,
                        deceleration = 16
                    },
                    yDestination = {
                        desiredPosition = 13 * 3200,
                        maxSpeed = 300,
                        acceleration = 16,
                        deceleration = 16
                    }
                }
            }
        },
    },
    protoMutators = {
    },
    protoButtons = {
        bts.momentarySwitch{
            attachment = {xOffset = 10 * 3200, yOffset = 12.75 * 3200},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 1,
        },
        bts.toggleSwitch{ -- Hard mode is momentarySwitch
            triggerID = 4,
            timeDirection = 'forwards',
            first = {
                attachment = {platform = nil, xOffset = 25 * 3200 - 800, yOffset = 12.25 * 3200},
                width = 800,
                height = 1600,
            },
            second = {
                attachment = {platform = nil, xOffset = 7 * 3200, yOffset = 12.25 * 3200},
                width = 800,
                height = 1600,
            }
        },
    },
    protoTriggerMods = {
        {
            triggerID = 3, -- Speed of time
            triggerClause = "o4 1 & 24 |",
        },
        {
            triggerID = 5, -- Paradox pressure
            triggerClause = "o4 4 & 100 |",
        },
    },
    protoGlitz = {
        bts.easyWireGlitz{
            x = {pos = 10.5 * 3200},
            y1 = {pos = 13 * 3200},
            y2 = {pos = 15.5 * 3200},
            triggerID = 1,
            useTriggerArrival = true
        },
        bts.easyWireGlitz{
            x1 = {pos = 10.5 * 3200},
            x2 = {pos = 14 * 3200},
            y = {pos = 15.5 * 3200},
            triggerID = 1,
            useTriggerArrival = true
        },
        bts.wireGlitz{
            x1 = {platform = nil, pos = 3.8 * 3200},
            y1 = {platform = nil, pos = 11.8 * 3200 },
            x2 = {platform = nil, pos = (3.8 + 2.6 + 0.2) * 3200},
            y2 = {platform = nil, pos = (11.8 + 1.2 + 0.2) * 3200},
            triggerClause = "o4 !",
            useTriggerArrival = false
        },
        bts.basicRectangleGlitz{
            x = 3.9 * 3200,
            y = 11.9 * 3200,
            width = 2.6 * 3200,
            height = 1.2 * 3200,
            layer = 1600,
            colour = {r = 120, g = 120, b = 120},
        },
        bts.basicTextGlitz{
            x = 4.1 * 3200,
            y = 12.2 * 3200,
            text = "Fast Time",
            size = 1600,
            layer = 1600,
            colour = {r = 255, g = 255, b = 255},
        },
        bts.wireGlitz{
            x1 = {platform = nil, pos = 25.4 * 3200},
            y1 = {platform = nil, pos = 11.8 * 3200 },
            x2 = {platform = nil, pos = (25.4 + 2.6 + 0.2) * 3200},
            y2 = {platform = nil, pos = (11.8 + 1.2 + 0.2) * 3200},
            triggerID = 4,
            useTriggerArrival = false
        },
        bts.basicRectangleGlitz{
            x = 25.5 * 3200,
            y = 11.9 * 3200,
            width = 2.6 * 3200,
            height = 1.2 * 3200,
            layer = 1600,
            colour = {r = 120, g = 120, b = 120},
        },
        bts.basicTextGlitz{
            x = 25.7 * 3200,
            y = 12.2 * 3200,
            text = "Slow Time",
            size = 1600,
            layer = 1600,
            colour = {r = 255, g = 255, b = 255},
        },
    },
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
