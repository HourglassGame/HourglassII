local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 24.5 * 3200, yOffset = 11 * 3200},
            index = 1,
            width = 2 * 3200,
            height = 2 * 3200,
            collisionOverlap = 50,
            timeDirection = 'reverse',
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
            width = 3200,
            height = 4 * 3200,
            timeDirection = 'reverse',
            lastStateTriggerID = 3,
            buttonTriggerID = 1,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 22 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 5 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 22 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 9 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                }
            }
        },
        {
            width = 3200,
            height = 3200,
            timeDirection = 'reverse',
            lastStateTriggerID = 4,
            triggerClause = "t1 !",
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 5 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 8 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 6 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 8 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                }
            }
        },
        {
            width = 3200,
            height = 3200,
            timeDirection = 'reverse',
            lastStateTriggerID = 5,
            triggerClause = "t1 !",
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
                        desiredPosition = 8 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 7 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 8 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                }
            }
        },
    },
    protoMutators = {
    },
    protoButtons = {
        bts.multiStickySwitch{
            buttons = {
                {
                    attachment = {platform = nil, xOffset = 11.25 * 3200, yOffset = 12.75 * 3200},
                    width = 1600,
                    height = 800,
                },
                {
                    attachment = {platform = nil, xOffset = 14.25 * 3200, yOffset = 12.75 * 3200},
                    width = 1600,
                    height = 800,
                },
                {
                    attachment = {platform = nil, xOffset = 17.25 * 3200, yOffset = 12.75 * 3200},
                    width = 1600,
                    height = 800,
                },
                {
                    attachment = {platform = nil, xOffset = 20.25 * 3200, yOffset = 12.75 * 3200},
                    width = 1600,
                    height = 800,
                },
            },
            timeDirection = 'reverse',
            triggerID = 1,
            stateTriggerID = 2
        },
    },
    protoGlitz = {
        bts.wireGlitz{
            x1 = {platform = nil, pos = 11.25 * 3200},
            y1 = {platform = nil, pos = 13 * 3200 },
            x2 = {platform = nil, pos = 20.75 * 3200},
            y2 = {platform = nil, pos = 13.2 * 3200},
            triggerID = 1,
            useTriggerArrival = false
        },
        bts.wireGlitz{
            x1 = {platform = nil, pos = 15.95 * 3200},
            y1 = {platform = nil, pos = 13 * 3200 },
            x2 = {platform = nil, pos = 16.05 * 3200},
            y2 = {platform = nil, pos = 14.1 * 3200},
            triggerID = 1,
            useTriggerArrival = false
        },
        bts.wireGlitz{
            x1 = {platform = nil, pos = 12.75 * 3200},
            y1 = {platform = nil, pos = 14 * 3200 },
            x2 = {platform = nil, pos = 22.55 * 3200},
            y2 = {platform = nil, pos = 14.1 * 3200},
            triggerID = 1,
            useTriggerArrival = false
        },
        bts.wireGlitz{
            x1 = {platform = nil, pos = 22.45 * 3200},
            y1 = {platform = nil, pos = 13 * 3200 },
            x2 = {platform = nil, pos = 22.55 * 3200},
            y2 = {platform = nil, pos = 14.1 * 3200},
            triggerID = 1,
            useTriggerArrival = false
        },
        bts.wireGlitz{
            x1 = {platform = nil, pos = 2.8 * 3200},
            y1 = {platform = nil, pos = 14 * 3200 },
            x2 = {platform = nil, pos = 12.75 * 3200},
            y2 = {platform = nil, pos = 14.1 * 3200},
            triggerClause = "t1 !",
            useTriggerArrival = false
        },
        bts.wireGlitz{
            x1 = {platform = nil, pos = 2.8 * 3200},
            y1 = {platform = nil, pos = 8.45 * 3200 },
            x2 = {platform = nil, pos = 2.9 * 3200},
            y2 = {platform = nil, pos = 14.1 * 3200},
            triggerClause = "t1 !",
            useTriggerArrival = false
        },
        bts.wireGlitz{
            x1 = {platform = nil, pos = 2.8 * 3200},
            y1 = {platform = nil, pos = 8.45 * 3200 },
            x2 = {platform = nil, pos = 6 * 3200},
            y2 = {platform = nil, pos = 8.55 * 3200},
            triggerClause = "t1 !",
            useTriggerArrival = false
        },
        --bts.basicTextGlitz{
        --    x = 15 * 3200,
        --    y = 1.25 * 3200,
        --    text = " <- Creates a box at the start\n                if the door is closed.",
        --    size = 1400,
        --    layer = 1600,
        --    colour = {r = 255, g = 255, b = 255},
        --},
        bts.basicRectangleGlitz{
            x = 12.55 * 3200,
            y = 13.85 * 3200,
            width = 0.4 * 3200,
            height = 0.4 * 3200,
            layer = 1600,
            colour = {r = 80, g = 80, b = 80},
        },
        bts.basicTextGlitz{
            x = 12.65 * 3200,
            y = 13.75 * 3200,
            text = "!",
            size = 1400,
            layer = 1600,
            colour = {r = 255, g = 255, b = 255},
        },
    }
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
