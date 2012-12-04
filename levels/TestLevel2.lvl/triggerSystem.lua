if (false) then


local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = 1, xOffset = 3 * 3200, yOffset = -2 * 3200},
            index = 1,
            width = 2 * 3200,
            height = 2 * 3200,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = 1,
            xDestination = 0,
            yDestination = 0,
            relativeTime = true,
            timeDestination = 0,
            illegalDestination = 2,
            fallable = true,
            winner = false
        },
        {
            attachment = {platform = nil, xOffset = 22 * 3200, yOffset = 16 * 3200},
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
            width = 6*3200,
            height = 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 2,
            buttonTriggerID = 1,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 15 * 3200,
                        maxSpeed = 250,
                        acceleration = 250,
                        deceleration = 250
                    },
                    yDestination = {
                        desiredPosition = 4 * 3200,
                        maxSpeed = 250,
                        acceleration = 250,
                        deceleration = 250
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 15 * 3200,
                        maxSpeed = 250,
                        acceleration = 250,
                        deceleration = 250
                    },
                    yDestination = {
                        desiredPosition = 18 * 3200,
                        maxSpeed = 250,
                        acceleration = 250,
                        deceleration = 250
                    }
                }
            }
        },
        {
            width = 3200,
            height = 2*3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 4,
            buttonTriggerID = 3,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 21 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 14 * 3200,
                        maxSpeed = 300,
                        acceleration = 20,
                        deceleration = 20
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 21 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 16 * 3200,
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
        bts.momentarySwitch{
            triggerID = 1,
            timeDirection = 'forwards',
            attachment = {platform = nil, xOffset = 9*3200, yOffset = 17.75 * 3200 },
            width = 3200,
            height = 800,
        },
        bts.momentarySwitch{
            triggerID = 3,
            timeDirection = 'forwards',
            attachment = {platform = nil, xOffset = 29*3200 - 800, yOffset = 4 * 3200 },
            width = 800,
            height = 3200,
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
function mutateObject(responsibleManipulatorIndices, dynamicObject)
    return dynamicObject
end

getDepartureInformation = bts.getDepartureInformation(tempStore)

-----------------------------
else ----- IF TRUE/FALSE ----
-----------------------------

--triggerArrivals have already had default values inserted by C++
--for trigger indices that did not arrive by the time this is called
function calculatePhysicsAffectingStuff(frameNumber, triggerArrivals)

	local retv = {
		additionalBoxes = {},
		collisions = {
			{
				x = 5*3200,
				y = 17*3200,
				xspeed = 0,
				yspeed = 0,
				width = 5*3200,
				height = 3200,
				timeDirection = 'forwards'
			}
		},
		portals = {
			{
				index = 1,
				x = 7*3200,
				y = 15*3200,
				width = 2 * 3200,
				height = 2 * 3200,
				xspeed = 0,
				yspeed = 500,
				collisionOverlap = 50,
				timeDirection = 'forwards',
				destinationIndex = 1,
				xDestination = 0,
				yDestination = 0,
				relativeTime = true,
				timeDestination = 0,
				illegalDestination = 2,
				fallable = true,
				winner = false
			}
		},
		mutators = {},
		arrivalLocations = {
			{
				x = 7*3200,
				y = 7*3200,
				xspeed = 0,
				yspeed = 500,
				timeDirection = 'forwards'
			},
			{
				x = 7*3200,
				y = 7*3200,
				xspeed = 0,
				yspeed = -3000,
				timeDirection = 'forwards'
			}
		},
	}
	
	return retv
end

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

function getDepartureInformation()
	return 
	{{},{},{},{}}, 
	{
		{
			layer = 500,
			x = 7*3200,
			y = 15*3200,
			width = 2 * 3200,
			height = 2 * 3200,
			colour = {r = 50, g = 0, b = 0}
		}
	},
    {
		{
			layer = 500,
			x = 7*3200,
			y = 15*3200,
			width = 2 * 3200,
			height = 2 * 3200,
			colour = {r = 0, g = 0, b = 50}
		}
	},
	{}
end

end
