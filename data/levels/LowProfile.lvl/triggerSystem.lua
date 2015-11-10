local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 16.5 * 3200, yOffset = 4 * 3200},
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
            width = 3200,
            height = 2*3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 6,
            triggerFunction = function(triggers)
                return (triggers[5][1] == 1 and triggers[4][1] == 1 and triggers[3][1] == 1 and triggers[2][1] == 1) or triggers[1][1] == 1
            end,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 14.5 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 2 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 14.5 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 4 * 3200,
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
        bts.stickySwitch{
            attachment = {platform = nil, xOffset = 18.75 * 3200, yOffset = 5 * 3200},
            width = 800,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 1
        },
        bts.stickySwitch{
            attachment = {platform = nil, xOffset = 3.75 * 3200, yOffset = 13.75 * 3200},
            width = 1600,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 2
        },
        bts.stickySwitch{
            attachment = {platform = nil, xOffset = 7.75 * 3200, yOffset = 13.75 * 3200},
            width = 1600,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 3
        },
        bts.stickySwitch{
            attachment = {platform = nil, xOffset = 13.75 * 3200, yOffset = 13.75 * 3200},
            width = 1600,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 4
        },
        bts.stickySwitch{
            attachment = {platform = nil, xOffset = 17.75 * 3200, yOffset = 13.75 * 3200},
            width = 1600,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 5
        },
    },
    protoGlitz = {
        -- Wires just below the buttons
        bts.wireGlitz{
            x1 = {platform = nil, pos = 3.95 * 3200},
            y1 = {platform = nil, pos = 14 * 3200 },
            x2 = {platform = nil, pos = 4.05 * 3200},
            y2 = {platform = nil, pos = 14.55 * 3200},
            triggerID = 2,
            useTriggerArrival = false
        },
        bts.wireGlitz{
            x1 = {platform = nil, pos = 7.95 * 3200},
            y1 = {platform = nil, pos = 14 * 3200 },
            x2 = {platform = nil, pos = 8.05 * 3200},
            y2 = {platform = nil, pos = 14.55 * 3200},
            triggerID = 3,
            useTriggerArrival = false
        },
        bts.wireGlitz{
            x1 = {platform = nil, pos = 13.95 * 3200},
            y1 = {platform = nil, pos = 14 * 3200 },
            x2 = {platform = nil, pos = 14.05 * 3200},
            y2 = {platform = nil, pos = 14.55 * 3200},
            triggerID = 4,
            useTriggerArrival = false
        },
        bts.wireGlitz{
            x1 = {platform = nil, pos = 17.95 * 3200},
            y1 = {platform = nil, pos = 14 * 3200 },
            x2 = {platform = nil, pos = 18.05 * 3200},
            y2 = {platform = nil, pos = 14.55 * 3200},
            triggerID = 5,
            useTriggerArrival = false
        },
        -- Wires running along the bottom of the screen
        bts.wireGlitz{
            x1 = {platform = nil, pos = 4 * 3200},
            y1 = {platform = nil, pos = 14.45 * 3200 },
            x2 = {platform = nil, pos = 8 * 3200},
            y2 = {platform = nil, pos = 14.55 * 3200},
            triggerID = 2,
            useTriggerArrival = false
        },
        bts.wireGlitz{
            x1 = {platform = nil, pos = 8 * 3200},
            y1 = {platform = nil, pos = 14.45 * 3200 },
            x2 = {platform = nil, pos = 14 * 3200},
            y2 = {platform = nil, pos = 14.55 * 3200},
            triggerFunction = function (triggerArrivals, outputTriggers)
                return triggerArrivals[2][1] == 1 and triggerArrivals[3][1] == 1
            end,
        },
        bts.wireGlitz{
            x1 = {platform = nil, pos = 14 * 3200},
            y1 = {platform = nil, pos = 14.45 * 3200 },
            x2 = {platform = nil, pos = 18 * 3200},
            y2 = {platform = nil, pos = 14.55 * 3200},
            triggerFunction = function (triggerArrivals, outputTriggers)
                return triggerArrivals[2][1] == 1 and triggerArrivals[3][1] == 1 and triggerArrivals[4][1] == 1
            end,
        },
        -- Logic boxes at the bottom of the screen
        bts.basicRectangleGlitz{
            x = 7.7 * 3200,
            y = 14.3 * 3200,
            width = 0.6 * 3200,
            height = 0.6 * 3200,
            layer = 1600,
            colour = {r = 120, g = 120, b = 120},
        },
        bts.basicTextGlitz{
            x = 7.85 * 3200,
            y = 14.3 * 3200,
            text = "&",
            size = 1400,
            layer = 1600,
            colour = {r = 255, g = 255, b = 255},
        },
        bts.basicRectangleGlitz{
            x = 13.7 * 3200,
            y = 14.3 * 3200,
            width = 0.6 * 3200,
            height = 0.6 * 3200,
            layer = 1600,
            colour = {r = 120, g = 120, b = 120},
        },
        bts.basicTextGlitz{
            x = 13.85 * 3200,
            y = 14.3 * 3200,
            text = "&",
            size = 1400,
            layer = 1600,
            colour = {r = 255, g = 255, b = 255},
        },
        bts.basicRectangleGlitz{
            x = 17.7 * 3200,
            y = 14.3 * 3200,
            width = 0.6 * 3200,
            height = 0.6 * 3200,
            layer = 1600,
            colour = {r = 120, g = 120, b = 120},
        },
        bts.basicTextGlitz{
            x = 17.85 * 3200,
            y = 14.3 * 3200,
            text = "&",
            size = 1400,
            layer = 1600,
            colour = {r = 255, g = 255, b = 255},
        },
        -- Wires leading up the right wall
        bts.wireGlitz{
            x1 = {platform = nil, pos = 18 * 3200},
            y1 = {platform = nil, pos = 14.45 * 3200 },
            x2 = {platform = nil, pos = 20.55 * 3200},
            y2 = {platform = nil, pos = 14.55 * 3200},
            triggerFunction = function (triggerArrivals, outputTriggers)
                return triggerArrivals[2][1] == 1 and triggerArrivals[3][1] == 1 and triggerArrivals[4][1] == 1 and triggerArrivals[5][1] == 1
            end,
        },
        bts.wireGlitz{
            x1 = {platform = nil, pos = 20.45 * 3200},
            y1 = {platform = nil, pos = 14.55 * 3200 },
            x2 = {platform = nil, pos = 20.55 * 3200},
            y2 = {platform = nil, pos = 7.5 * 3200},
            triggerFunction = function (triggerArrivals, outputTriggers)
                return triggerArrivals[2][1] == 1 and triggerArrivals[3][1] == 1 and triggerArrivals[4][1] == 1 and triggerArrivals[5][1] == 1
            end,
        },
        -- Right wall OR box
        bts.basicRectangleGlitz{
            x = 20.2 * 3200,
            y = 7.3 * 3200,
            width = 0.6 * 3200,
            height = 0.6 * 3200,
            layer = 1600,
            colour = {r = 120, g = 120, b = 120},
        },
        bts.basicTextGlitz{
            x = 20.3 * 3200,
            y = 7.3 * 3200,
            text = "or",
            size = 1400,
            layer = 1600,
            colour = {r = 255, g = 255, b = 255},
        },
        -- Wire loop from top right button
        bts.wireGlitz{
            x1 = {platform = nil, pos = 19 * 3200},
            y1 = {platform = nil, pos = 5.2 * 3200 },
            x2 = {platform = nil, pos = 20.55 * 3200},
            y2 = {platform = nil, pos = 5.3 * 3200},
            triggerID = 1,
            useTriggerArrival = false
        },
        bts.wireGlitz{
            x1 = {platform = nil, pos = 20.45 * 3200},
            y1 = {platform = nil, pos = 5.2 * 3200 },
            x2 = {platform = nil, pos = 20.55 * 3200},
            y2 = {platform = nil, pos = 7.5 * 3200},
            triggerID = 1,
            useTriggerArrival = false
        },
        bts.wireGlitz{
            x1 = {platform = nil, pos = 14.95 * 3200},
            y1 = {platform = nil, pos = 7.45 * 3200 },
            x2 = {platform = nil, pos = 20.5 * 3200},
            y2 = {platform = nil, pos = 7.55 * 3200},
            triggerFunction = function (triggerArrivals, outputTriggers)
                return (triggerArrivals[2][1] == 1 and triggerArrivals[3][1] == 1 and 
                triggerArrivals[4][1] == 1 and triggerArrivals[5][1] == 1) or triggerArrivals[1][1] == 1
            end,
        },
        bts.wireGlitz{
            x1 = {platform = nil, pos = 14.95 * 3200},
            y1 = {platform = nil, pos = 7.45 * 3200 },
            x2 = {platform = nil, pos = 15.05 * 3200},
            y2 = {platform = nil, pos = 6 * 3200},
            triggerFunction = function (triggerArrivals, outputTriggers)
                return (triggerArrivals[2][1] == 1 and triggerArrivals[3][1] == 1 and 
                triggerArrivals[4][1] == 1 and triggerArrivals[5][1] == 1) or triggerArrivals[1][1] == 1
            end,
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
