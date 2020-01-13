local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {xOffset = 2125 * 32, yOffset = 7 * 3200},
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
            illegalDestination = 1,
            fallable = false,
            winner = true
        },
        {
            attachment = {platform = 2, xOffset = 0 * 3200, yOffset = -4 * 3200},
            index = 2,
            width = 1600,
            height = 9600,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = nil,
            xDestination = 0,
            yDestination = 0,
            relativeTime = true,
            timeDestination = -600,
            illegalDestination = 2,
            fallable = true,
            winner = false,
            triggerFunction = function (triggers)
                return triggers[4][1] > 0
            end,
        }
    },
    protoCollisions = {
        {
            width = 1 * 3200,
            height = 3 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 2,
            buttonTriggerID = 4,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 195 * 320,
                        maxSpeed = 200,
                        acceleration = 20,
                        deceleration = 18
                    },
                    yDestination = {
                        desiredPosition = 3 * 3200,
                        maxSpeed = 300,
                        acceleration = 20,
                        deceleration = 18
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 195 * 320,
                        maxSpeed = 200,
                        acceleration = 20,
                        deceleration = 18
                    },
                    yDestination = {
                        desiredPosition = 6 * 3200,
                        maxSpeed = 300,
                        acceleration = 20,
                        deceleration = 18
                    }
                }
            }
        },
        {
            lastStateTriggerID = 3,
            rawCollisionFunction = function(triggerArrivals, outputTriggers, frameNumber)
                -- 600 frames to sweep map (10 seconds)
                return {
                        x = -frameNumber%600 * 128 + 1600,
                        y = 10 * 3200,
                        xspeed = -128,
                        yspeed = 0,
                        prevXspeed = -128,
                        prevYspeed = 0,
                        width = 1500,
                        height = 2000,
                        timeDirection = 'forwards'
                    }
                
            end
        },
    },
    protoMutators = {
    },
    protoButtons = {
        bts.stickySwitch{
            attachment = {xOffset = 975 * 32, yOffset = 875 * 32},
            width = 1600,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 1,
        },
    },
    protoGlitz = {
    },
    triggerManipulationFunction = function (triggerArrivals, outputTriggers, frameNumber)
        if triggerArrivals[1][1] > 0 and (frameNumber%600 == 116 or triggerArrivals[4][1] > 0) then
            outputTriggers[4] = {triggerArrivals[4][1] == 0 and 1 or 2}
        end
    end
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
