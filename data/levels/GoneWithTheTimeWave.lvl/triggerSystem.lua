local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {xOffset = 21.25 * 3200, yOffset = 7 * 3200},
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
            triggerClause = "t1",
            --triggerFunction = function (triggers)
            --    return triggers[1][1] > 0
            --end,
        }
    },
    protoCollisions = {
        {
            width = 1 * 3200,
            height = 3 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 2,
            buttonTriggerID = 1,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 19.5 * 3200,
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
                        desiredPosition = 19.5 * 3200,
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
            width = 1 * 3200,
            height = 1 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 3,
            triggerClause = "t1 t4 |",
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = -19.75 * 3200,
                        maxSpeed = 128,
                        acceleration = 128,
                        deceleration = 128
                    },
                    yDestination = {
                        desiredPosition = 10 * 3200,
                        maxSpeed = 128,
                        acceleration = 128,
                        deceleration = 128
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 19.75 * 3200,
                        maxSpeed = 128,
                        acceleration = 128,
                        deceleration = 128
                    },
                    yDestination = {
                        desiredPosition = 10 * 3200,
                        maxSpeed = 128,
                        acceleration = 128,
                        deceleration = 128
                    }
                }
            }
        },
        --{
        --    lastStateTriggerID = 3,
        --    rawCollisionFunction = function(triggerArrivals, outputTriggers)
        --        if triggerArrivals[1][1] == 0 and triggerArrivals[4][1] == 0 then
        --            return {
        --                x = 19.75 * 3200,
        --                y = 10 * 3200,
        --                xspeed = 0,
        --                yspeed = 0,
        --                prevXspeed = 0,
        --                prevYspeed = 0,
        --                width = 1600,
        --                height = 2000,
        --                timeDirection = 'forwards'
        --            }
        --        end
        --        
        --        -- 600 frames to sweep map (10 seconds)
        --        local x = triggerArrivals[3][1]
        --        x = x - 128
        --        if x < 1600 then
        --            x = x + 24 * 3200
        --        end
        --        return {
        --                x = x,
        --                y = 10 * 3200,
        --                xspeed = -128,
        --                yspeed = 0,
        --                prevXspeed = -128,
        --                prevYspeed = 0,
        --                width = 1500,
        --                height = 2000,
        --                timeDirection = 'forwards'
        --            }
        --        
        --    end
        --},
    },
    protoMutators = {
    },
    protoButtons = {
        bts.stickySwitch{
            attachment = {xOffset = 9.25 * 3200, yOffset = 8.75 * 3200},
            width = 1600,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 1,
            extraTriggerIDs = {4}
        },
    },
    protoGlitz = {
    },
    protoTriggerMods = {
        {
            triggerID = 3,
            triggerClause = "o3,1 1600 < o3,1 24 3200 * + & o3,1 |",
        }
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
