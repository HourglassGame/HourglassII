local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 9.5 * 3200, yOffset = 9 * 3200},
            index = 1,
            width = 2 * 3200,
            height = 2 * 3200,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = 1,
            xDestination = 0,
            yDestination = -6*3200,
            relativeTime = false,
            timeDestination = 0,
            illegalDestination = 1,
            fallable = false,
            winner = false
        },
        {
            attachment = {platform = nil, xOffset = 9.5 * 3200, yOffset = 2 * 3200},
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
            width = 3*3200,
            height = 2*3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 3,
            buttonTriggerID = 6,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 9 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
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
                        desiredPosition = 9 * 3200,
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
		{
            width = 0.5 * 3200,
            height = 3*3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 7,
            buttonTriggerID = 9,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 8.5 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 11 * 3200,
                        maxSpeed = 300,
                        acceleration = 20,
                        deceleration = 20
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 8.5 * 3200,
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
            width = 0.5 * 3200,
            height = 3*3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 8,
            buttonTriggerID = 9,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 12 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 11 * 3200,
                        maxSpeed = 300,
                        acceleration = 20,
                        deceleration = 20
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 12 * 3200,
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
        bts.pickup{
            attachment = {platform = nil, xOffset = 17.25 * 3200, yOffset = 9.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 4,
			pickupType = 'timeJump'
        },
		bts.pickup{
            attachment = {platform = nil, xOffset = 3.25 * 3200, yOffset = 9.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 5,
			pickupType = 'timeJump'
        }
    },
    protoButtons = {
        bts.stickySwitch{
            attachment = {platform = nil, xOffset = 7.4 * 3200, yOffset = 6 * 3200},
			width = 0.2 * 3200,
			height = 3200,
            timeDirection = 'forwards',
            triggerID = 1
        },
		bts.stickySwitch{
            attachment = {platform = nil, xOffset = 13.4 * 3200, yOffset = 6 * 3200},
			width = 0.2 * 3200,
			height = 3200,
            timeDirection = 'forwards',
            triggerID = 2
        },
        bts.multiStickySwitch{
            buttons = {
				{
					attachment = {platform = nil, xOffset = 6.75 * 3200, yOffset = 10.75 * 3200},
					width = 3200,
					height = 800,
				},
				{
					attachment = {platform = nil, xOffset = 13.25 * 3200, yOffset = 10.75 * 3200},
					width = 3200,
					height = 800,
				},
			},
            timeDirection = 'forwards',
            triggerID = 9
        },
    }
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

--getDepartureInformation = bts.getDepartureInformation(tempStore)

function getDepartureInformation(departures)
	bts.getDepartureInformation(tempStore)(departures)
	
	tempStore.outputTriggers[6][1] = tempStore.outputTriggers[1][1]*(1 - tempStore.outputTriggers[2][1])
	
	return tempStore.outputTriggers, tempStore.forwardsGlitz, tempStore.reverseGlitz, tempStore.additionalEndBoxes
end

