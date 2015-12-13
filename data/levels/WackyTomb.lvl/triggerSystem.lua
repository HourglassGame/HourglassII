local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 19.5 * 3200, yOffset = 4 * 3200},
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
                return (triggers[5][1] == 1 and triggers[4][1] == 1) or triggers[3][1] == 1
            end,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 17 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 1 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 17 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 3 * 3200,
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
            attachment = {platform = nil, xOffset = 21.75 * 3200, yOffset = 5 * 3200},
            width = 800,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 3
        },
        bts.stickySwitch{
            attachment = {platform = nil, xOffset = 8.75 * 3200, yOffset = 13.75 * 3200},
            width = 1600,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 4
        },
        bts.stickySwitch{
            attachment = {platform = nil, xOffset = 14.75 * 3200, yOffset = 13.75 * 3200},
            width = 1600,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 5
        },
    },
    protoGlitz = {
        bts.easyWireGlitz{
            x = {platform = nil, pos = 9 * 3200},
            y1 = {platform = nil, pos = 14 * 3200},
            y2 = {platform = nil, pos = 14.55 * 3200},
            triggerID = 4,
            useTriggerArrival = false
        },
        bts.easyWireGlitz{
            x1 = {platform = nil, pos = 9 * 3200},
            x2 = {platform = nil, pos = 15 * 3200},
            y = {platform = nil, pos = 14.5 * 3200},
            triggerID = 4,
            useTriggerArrival = false
        },
        bts.easyWireGlitz{
            x = {platform = nil, pos = 15 * 3200},
            y1 = {platform = nil, pos = 14 * 3200},
            y2 = {platform = nil, pos = 14.55 * 3200},
            triggerID = 5,
            useTriggerArrival = false
        },
        bts.basicRectangleGlitz{
            x = 14.8 * 3200,
            y = 14.3 * 3200,
            width = 0.4 * 3200,
            height = 0.4 * 3200,
            layer = 1600,
            colour = {r = 80, g = 80, b = 80},
        },
        bts.basicTextGlitz{
            x = 14.85 * 3200,
            y = 14.2 * 3200,
            text = "&",
            size = 1400,
            layer = 1600,
            colour = {r = 255, g = 255, b = 255},
        },
        bts.easyWireGlitz{
            x1 = {platform = nil, pos = 15 * 3200},
            x2 = {platform = nil, pos = 17.55 * 3200},
            y = {platform = nil, pos = 14.5 * 3200},
            triggerFunction = function(triggerArrivals, outputTriggers) return triggerArrivals[4][1]*triggerArrivals[5][1] == 1 end,
            useTriggerArrival = false
        },
        bts.easyWireGlitz{
            x = {platform = nil, pos = 17.5 * 3200},
            y1 = {platform = nil, pos = 14.5 * 3200},
            y2 = {platform = nil, pos = 6.5 * 3200},
            triggerFunction = function(triggerArrivals, outputTriggers) return triggerArrivals[4][1]*triggerArrivals[5][1] == 1 end,
            useTriggerArrival = false
        },
        bts.basicRectangleGlitz{
            x = 17.3 * 3200,
            y = 6.3 * 3200,
            width = 0.4 * 3200,
            height = 0.4 * 3200,
            layer = 1600,
            colour = {r = 80, g = 80, b = 80},
        },
        bts.basicTextGlitz{
            x = 17.3 * 3200,
            y = 6.2 * 3200,
            text = "or",
            size = 1400,
            layer = 1600,
            colour = {r = 255, g = 255, b = 255},
        },
        bts.easyWireGlitz{
            x1 = {platform = nil, pos = 22 * 3200},
            x2 = {platform = nil, pos = 22.55 * 3200},
            y = {platform = nil, pos = 5.25 * 3200},
            triggerID = 3,
            useTriggerArrival = false
        },
        bts.easyWireGlitz{
            x = {platform = nil, pos = 22.5 * 3200},
            y1 = {platform = nil, pos = 5.25 * 3200},
            y2 = {platform = nil, pos = 6.55 * 3200},
            triggerID = 3,
            useTriggerArrival = false
        },
        bts.easyWireGlitz{
            x1 = {platform = nil, pos = 17.5 * 3200},
            x2 = {platform = nil, pos = 22.5 * 3200},
            y = {platform = nil, pos = 6.5 * 3200},
            triggerID = 3,
            useTriggerArrival = false
        },
        bts.easyWireGlitz{
            x = {platform = nil, pos = 17.5 * 3200},
            y1 = {platform = nil, pos = 5 * 3200},
            y2 = {platform = nil, pos = 6.5 * 3200},
            y2 = {platform = nil, pos = 6.5 * 3200},
            triggerFunction = function(triggerArrivals, outputTriggers) 
                return (triggerArrivals[4][1] == 1 and triggerArrivals[5][1] == 1) or triggerArrivals[3][1] == 1
            end,
            useTriggerArrival = false
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
