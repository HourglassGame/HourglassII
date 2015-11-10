local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {xOffset = 14.5 * 3200, yOffset = 6 * 3200},
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
            illegalDestination = 1,
            fallable = false,
            winner = true
        },
    },
    protoCollisions = {
        {
            width = 3*3200,
            height = 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 4,
            triggerFunction = function(triggers)
                return triggers[1][1] == 1 or triggers[2][1] == 1
            end,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 3 * 3200,
                        maxSpeed = 200,
                        acceleration = 20,
                        deceleration = 20
                    },
                    yDestination = {
                        desiredPosition = 5 * 3200,
                        maxSpeed = 300,
                        acceleration = 20,
                        deceleration = 20
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 8 * 3200,
                        maxSpeed = 200,
                        acceleration = 20,
                        deceleration = 20
                    },
                    yDestination = {
                        desiredPosition = 5 * 3200,
                        maxSpeed = 300,
                        acceleration = 20,
                        deceleration = 20
                    }
                }
            }
        },
    },
    protoMutators = {
        bts.pickup{
            attachment = {xOffset = 10.75 * 3200, yOffset = 7.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 5,
            pickupType = 'timeGun'
        },
        bts.pickup{
            attachment = {xOffset = 11.75 * 3200, yOffset = 7.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 6,
            pickupType = 'timeJump'
        },
    },
    protoButtons = {
        bts.stickySwitch{
            attachment = {xOffset = 4.25 * 3200, yOffset = 9.75 * 3200},
            width = 1600,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 3,
            extraTriggerIDs = {1,2}
        },
    },
    protoGlitz = {
        bts.easyWireGlitz{
            x = {pos = 4.5 * 3200},
            y1 = {pos = 10 * 3200},
            y2 = {pos = 11.55 * 3200},
            triggerID = 3,
            useTriggerArrival = false
        },
        bts.easyWireGlitz{
            x1 = {pos = 4.5 * 3200},
            x2 = {pos = 8.5 * 3200},
            y = {pos = 11.5 * 3200},
            triggerID = 3,
            useTriggerArrival = false
        },
        bts.easyWireGlitz{
            x = {pos = 6.5 * 3200},
            y1 = {pos = 10.5 * 3200},
            y2 = {pos = 11.55 * 3200},
            triggerID = 3,
            useTriggerArrival = false
        },
        bts.easyWireGlitz{
            x = {pos = 8.5 * 3200},
            y1 = {pos = 10.5 * 3200},
            y2 = {pos = 11.55 * 3200},
            triggerID = 3,
            useTriggerArrival = false
        },
        bts.easyWireGlitz{
            x1 = {pos = 6.5 * 3200},
            x2 = {pos = 7.5 * 3200},
            y = {pos = 10.5 * 3200},
            triggerID = 1,
            useTriggerArrival = true
        },
        bts.easyWireGlitz{
            x1 = {pos = 7.5 * 3200},
            x2 = {pos = 8.5 * 3200},
            y = {pos = 10.5 * 3200},
            triggerID = 2,
            useTriggerArrival = true
        },
        bts.easyWireGlitz{
            x = {pos = 7.5 * 3200},
            y1 = {pos = 5.45 * 3200},
            y2 = {pos = 10.55 * 3200},
            triggerFunction = function (triggerArrivals, outputTriggers)
                return triggerArrivals[1][1] == 1 or triggerArrivals[2][1] == 1
            end,
        },
        bts.basicRectangleGlitz{
            x = 6.2 * 3200,
            y = 10.3 * 3200,
            width = 0.6 * 3200,
            height = 0.6 * 3200,
            layer = 1600,
            colour = {r = 120, g = 120, b = 120},
        },
        bts.basicTextGlitz{
            x = 6.1 * 3200,
            y = 9.8 * 3200,
            text = "-10",
            size = 1400,
            layer = 1600,
            colour = {r = 255, g = 255, b = 255},
        },
        bts.basicRectangleGlitz{
            x = 8.2 * 3200,
            y = 10.3 * 3200,
            width = 0.6 * 3200,
            height = 0.6 * 3200,
            layer = 1600,
            colour = {r = 120, g = 120, b = 120},
        },
        bts.basicTextGlitz{
            x = 8.1 * 3200,
            y = 9.8 * 3200,
            text = "-15",
            size = 1400,
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
