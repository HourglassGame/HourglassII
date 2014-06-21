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
	--	bts.pickup{
	--		attachment = {platform = nil, xOffset = 8.75 * 3200, yOffset = 12.75 * 3200},
	--		width = 1600,
	--		height = 1600,
	--		timeDirection = 'forwards',
	--		triggerID = 1,
	--	pickupType = 'timeJump'
	--	},
	--ts.pickup{
	--		attachment = {platform = nil, xOffset = 14.75 * 3200, yOffset = 12.75 * 3200},
	--		width = 1600,
	--		height = 1600,
	--		timeDirection = 'forwards',
	--		triggerID = 2,
	--	pickupType = 'timeJump'
	--	},
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
