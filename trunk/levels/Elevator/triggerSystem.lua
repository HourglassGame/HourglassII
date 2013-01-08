local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 4 * 3200, yOffset = 2 * 3200},
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
            width = 3*3200,
            height = 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 2,
            buttonTriggerID = 1,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 10 * 3200,
                        maxSpeed = 200,
                        acceleration = 20,
                        deceleration = 20
                    },
                    yDestination = {
                        desiredPosition = 4 * 3200,
                        maxSpeed = 300,
                        acceleration = 20,
                        deceleration = 20
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 10 * 3200,
                        maxSpeed = 200,
                        acceleration = 20,
                        deceleration = 20
                    },
                    yDestination = {
                        desiredPosition = 12 * 3200,
                        maxSpeed = 300,
                        acceleration = 20,
                        deceleration = 20
                    }
                }
            }
        },
    },
    protoMutators = {
    },
    protoButtons = {
        bts.momentarySwitch{
            triggerID = 1,
            timeDirection = 'forwards',
            attachment = {platform = 1, xOffset = 3*3200 - 800, yOffset = -1 * 3200 },
            width = 800,
            height = 3200,
        },
    },
	protoGlitz = {
		bts.wire{
			x1 = {platform = 1, pos = 3.55 * 3200},
			y1 = {platform = 1, pos = -0.55 * 3200 },
			x2 = {platform = 1, pos = 3 * 3200},
			y2 = {platform = 1, pos = -0.45 * 3200},
			triggerID = 1,
			useTriggerArrival = false
		},
		bts.wire{
			x1 = {platform = 1, pos = 3.45 * 3200},
			y1 = {platform = 1, pos = -0.55 * 3200 },
			x2 = {platform = nil, pos = 13.55 * 3200},
			y2 = {platform = nil, pos = 13.55 * 3200},
			triggerID = 1,
			useTriggerArrival = false
		},
		bts.wire{
			x1 = {platform = nil, pos = 13.55 * 3200},
			y1 = {platform = nil, pos = 13.45 * 3200 },
			x2 = {platform = nil, pos = 12.5 * 3200},
			y2 = {platform = nil, pos = 13.55 * 3200},
			triggerID = 1,
			useTriggerArrival = false
		},
		bts.wire{
			x1 = {platform = nil, pos = 11.45 * 3200},
			y1 = {platform = nil, pos = 13.45 * 3200 },
			x2 = {platform = nil, pos = 12.6 * 3200},
			y2 = {platform = nil, pos = 13.55 * 3200},
			triggerID = 1,
			useTriggerArrival = true
		},
		bts.wire{
			x1 = {platform = nil, pos = 11.45 * 3200},
			y1 = {platform = nil, pos = 13 * 3200 },
			x2 = {platform = nil, pos = 11.55 * 3200},
			y2 = {platform = nil, pos = 13.55 * 3200},
			triggerID = 1,
			useTriggerArrival = true
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
function mutateObject(responsibleManipulatorIndices, dynamicObject)
    return dynamicObject
end

getDepartureInformation = bts.getDepartureInformation(tempStore)
