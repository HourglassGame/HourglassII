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
            yDestination = -7*3200,
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
					attachment = {platform = nil, xOffset = 7 * 3200, yOffset = 10.75 * 3200},
					width = 1600,
					height = 800,
				},
				{
					attachment = {platform = nil, xOffset = 13.5 * 3200, yOffset = 10.75 * 3200},
					width = 1600,
					height = 800,
				},
			},
            timeDirection = 'forwards',
            triggerID = 9
        },
    },
	protoGlitz = {
		bts.wireGlitz{
			x1 = {platform = nil, pos = 7.45 * 3200},
			y1 = {platform = nil, pos = 5.45 * 3200},
			x2 = {platform = nil, pos = 7.55 * 3200},
			y2 = {platform = nil, pos = 6 * 3200},
			triggerID = 1,
			useTriggerArrival = false
		},
		bts.wireGlitz{
			x1 = {platform = nil, pos = 7.45 * 3200},
			y1 = {platform = nil, pos = 5.45 * 3200},
			x2 = {platform = nil, pos = 8.55 * 3200},
			y2 = {platform = nil, pos = 5.55 * 3200},
			triggerID = 1,
			useTriggerArrival = false
		},
		bts.wireGlitz{
			x1 = {platform = nil, pos = 8.55 * 3200},
			y1 = {platform = nil, pos = 5.55 * 3200},
			x2 = {platform = nil, pos = 8.45 * 3200},
			y2 = {platform = nil, pos = 4.45 * 3200},
			triggerID = 1,
			useTriggerArrival = false
		},
		bts.wireGlitz{
			x1 = {platform = nil, pos = 8.45 * 3200},
			y1 = {platform = nil, pos = 4.45 * 3200},
			x2 = {platform = nil, pos = 9.55 * 3200},
			y2 = {platform = nil, pos = 4.55 * 3200},
			triggerID = 1,
			useTriggerArrival = false
		},
		bts.wireGlitz{
			x1 = {platform = nil, pos = 7.45 * 3200},
			y1 = {platform = nil, pos = 5.45 * 3200},
			x2 = {platform = nil, pos = 8.55 * 3200},
			y2 = {platform = nil, pos = 5.55 * 3200},
			triggerID = 1,
			useTriggerArrival = false
		},
		bts.wireGlitz{
			x1 = {platform = nil, pos = 13.45 * 3200},
			y1 = {platform = nil, pos = 5.45 * 3200},
			x2 = {platform = nil, pos = 13.55 * 3200},
			y2 = {platform = nil, pos = 6 * 3200},
			triggerID = 2,
			useTriggerArrival = false
		},
		bts.wireGlitz{
			x1 = {platform = nil, pos = 13.55 * 3200},
			y1 = {platform = nil, pos = 5.45 * 3200},
			x2 = {platform = nil, pos = 12.45 * 3200},
			y2 = {platform = nil, pos = 5.55 * 3200},
			triggerID = 2,
			useTriggerArrival = false
		},
		bts.wireGlitz{
			x1 = {platform = nil, pos = 12.55 * 3200},
			y1 = {platform = nil, pos = 5.55 * 3200},
			x2 = {platform = nil, pos = 12.45 * 3200},
			y2 = {platform = nil, pos = 4.45 * 3200},
			triggerID = 2,
			useTriggerArrival = false
		},
		bts.wireGlitz{
			x1 = {platform = nil, pos = 12.55 * 3200},
			y1 = {platform = nil, pos = 4.55 * 3200},
			x2 = {platform = nil, pos = 9.45 * 3200},
			y2 = {platform = nil, pos = 4.45 * 3200},
			triggerFunction = function(triggerArrivals, outputTriggers) return triggerArrivals[2][1] == 0 end,
			useTriggerArrival = false
		},
		bts.wireGlitz{
			x1 = {platform = nil, pos = 9.45 * 3200},
			y1 = {platform = nil, pos = 4.45 * 3200},
			x2 = {platform = nil, pos = 9.55 * 3200},
			y2 = {platform = nil, pos = 5.55 * 3200},
			triggerFunction = function(triggerArrivals, outputTriggers) return triggerArrivals[1][1]*(1-triggerArrivals[2][1]) == 1 end,
			useTriggerArrival = false
		},
		bts.wireGlitz{
			x1 = {platform = nil, pos = 9.45 * 3200},
			y1 = {platform = nil, pos = 5.45 * 3200},
			x2 = {platform = nil, pos = 10.55 * 3200},
			y2 = {platform = nil, pos = 5.55 * 3200},
			triggerFunction = function(triggerArrivals, outputTriggers) return triggerArrivals[1][1]*(1-triggerArrivals[2][1]) == 1 end,
			useTriggerArrival = false
		},
		bts.wireGlitz{
			x1 = {platform = nil, pos = 10.45 * 3200},
			y1 = {platform = nil, pos = 5.45 * 3200},
			x2 = {platform = nil, pos = 10.55 * 3200},
			y2 = {platform = nil, pos = 6 * 3200},
			triggerID = 6,
			useTriggerArrival = true
		},
		bts.basicRectangleGlitz{
			x = 12.3 * 3200,
			y = 4.3 * 3200,
			width = 0.4 * 3200,
			height = 0.4 * 3200,
			layer = 1600,
			colour = {r = 80, g = 80, b = 80},
		},
		bts.basicTextGlitz{
			x = 12.45 * 3200,
			y = 4.2 * 3200,
			text = "!",
			size = 1400,
			layer = 1600,
			colour = {r = 255, g = 255, b = 255},
		},
		bts.basicRectangleGlitz{
			x = 9.3 * 3200,
			y = 4.3 * 3200,
			width = 0.4 * 3200,
			height = 0.4 * 3200,
			layer = 1600,
			colour = {r = 80, g = 80, b = 80},
		},
		bts.basicTextGlitz{
			x = 9.35 * 3200,
			y = 4.2 * 3200,
			text = "&",
			size = 1400,
			layer = 1600,
			colour = {r = 255, g = 255, b = 255},
		},
		bts.basicRectangleGlitz{
			x = 10.2 * 3200,
			y = 5.2 * 3200,
			width = 0.6 * 3200,
			height = 0.6 * 3200,
			layer = 1600,
			colour = {r = 120, g = 120, b = 120},
		},
		bts.basicTextGlitz{
			x = 10.3 * 3200,
			y = 4.7 * 3200,
			text = "-5",
			size = 1400,
			layer = 1600,
			colour = {r = 255, g = 255, b = 255},
		},
		bts.wireGlitz{
			x1 = {platform = nil, pos = 7.2 * 3200},
			y1 = {platform = nil, pos = 11.45 * 3200},
			x2 = {platform = nil, pos = 13.8 * 3200},
			y2 = {platform = nil, pos = 11.55 * 3200},
			triggerID = 9,
			useTriggerArrival = false
		},
		bts.wireGlitz{
			x1 = {platform = nil, pos = 7.2 * 3200},
			y1 = {platform = nil, pos = 11 * 3200},
			x2 = {platform = nil, pos = 7.3 * 3200},
			y2 = {platform = nil, pos = 11.55 * 3200},
			triggerID = 9,
			useTriggerArrival = false
		},
		bts.wireGlitz{
			x1 = {platform = nil, pos = 8.7 * 3200},
			y1 = {platform = nil, pos = 11 * 3200},
			x2 = {platform = nil, pos = 8.8 * 3200},
			y2 = {platform = nil, pos = 11.55 * 3200},
			triggerID = 9,
			useTriggerArrival = false
		},
		bts.wireGlitz{
			x1 = {platform = nil, pos = 12.2 * 3200},
			y1 = {platform = nil, pos = 11 * 3200},
			x2 = {platform = nil, pos = 12.3 * 3200},
			y2 = {platform = nil, pos = 11.55 * 3200},
			triggerID = 9,
			useTriggerArrival = false
		},
		bts.wireGlitz{
			x1 = {platform = nil, pos = 13.7 * 3200},
			y1 = {platform = nil, pos = 11 * 3200},
			x2 = {platform = nil, pos = 13.8 * 3200},
			y2 = {platform = nil, pos = 11.55 * 3200},
			triggerID = 9,
			useTriggerArrival = false
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

--getDepartureInformation = bts.getDepartureInformation(tempStore)

function getDepartureInformation(departures)
	bts.getDepartureInformation(tempStore)(departures)
	
	tempStore.outputTriggers[6][1] = tempStore.outputTriggers[1][1]*(1 - tempStore.outputTriggers[2][1])
	
	return tempStore.outputTriggers, tempStore.forwardsGlitz, tempStore.reverseGlitz, tempStore.additionalEndBoxes
end

