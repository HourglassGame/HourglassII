local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 15 * 3200, yOffset = 5 * 3200},
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
        },
    },
    protoCollisions = {
        {
            width = 4 * 3200,
            height = 1 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 6,
            buttonTriggerID = 1,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 14 * 3200,
                        maxSpeed = 250,
                        acceleration = 16,
                        deceleration = 16
                    },
                    yDestination = {
                        desiredPosition = 7 * 3200,
                        maxSpeed = 250,
                        acceleration = 16,
                        deceleration = 16
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 14 * 3200,
                        maxSpeed = 250,
                        acceleration = 16,
                        deceleration = 16
                    },
                    yDestination = {
                        desiredPosition = 14 * 3200,
                        maxSpeed = 250,
                        acceleration = 16,
                        deceleration = 16
                    }
                }
            }
        },
        {
            width = 2 * 3200,
            height = 1 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 8,
            triggerClause = "t4 t5 &",
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 12 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    },
                    yDestination = {
                        desiredPosition = 7 * 3200,
                        maxSpeed = 250,
                        acceleration = 18,
                        deceleration = 18
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 14 * 3200,
                        maxSpeed = 250,
                        acceleration = 18,
                        deceleration = 18
                    },
                    yDestination = {
                        desiredPosition = 7 * 3200,
                        maxSpeed = 250,
                        acceleration = 18,
                        deceleration = 18
                    }
                }
            }
        },
        {
            width = 2 * 3200,
            height = 1 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 9,
            triggerClause = "t4 t5 &",
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 18 * 3200,
                        maxSpeed = 250,
                        acceleration = 40,
                        deceleration = 40
                    },
                    yDestination = {
                        desiredPosition = 7 * 3200,
                        maxSpeed = 250,
                        acceleration = 18,
                        deceleration = 18
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 16 * 3200,
                        maxSpeed = 250,
                        acceleration = 18,
                        deceleration = 18
                    },
                    yDestination = {
                        desiredPosition = 7 * 3200,
                        maxSpeed = 250,
                        acceleration = 18,
                        deceleration = 18
                    }
                }
            }
        },
    },
    protoMutators = {
        bts.pickup{
            attachment = {platform = nil, xOffset = 4.75 * 3200, yOffset = 4.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 11,
            pickupType = 'timeReverse'
        },
        bts.pickup{
            attachment = {platform = nil, xOffset = 26.75 * 3200, yOffset = 4.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 12,
            pickupType = 'timeReverse'
        },
        bts.pickup{
            attachment = {platform = nil, xOffset = 15.75 * 3200, yOffset = 11.75 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 7,
            pickupType = 'timeJump'
        },
    },
    protoButtons = {
        bts.momentarySwitch{
            triggerID = 1,
            timeDirection = 'forwards',
            attachment = {platform = 1, xOffset = 1.5 * 3200, yOffset = -0.25 * 3200 },
            width = 3200,
            height = 800,
        },
        bts.stickySwitch{
            triggerID = 2,
            timeDirection = 'forwards',
            attachment = {platform = nil, xOffset = 13.25 * 3200, yOffset = 12 * 3200 },
            width = 1600,
            height = 800,
        },
        bts.stickySwitch{
            triggerID = 3,
            timeDirection = 'forwards',
            attachment = {platform = nil, xOffset = 18.25 * 3200, yOffset = 12 * 3200 },
            width = 1600,
            height = 800,
        },
        bts.stickySwitch{
            triggerID = 4,
            timeDirection = 'forwards',
            attachment = {platform = nil, xOffset = 3 * 3200, yOffset = 7.25 * 3200 },
            width = 800,
            height = 1600,
        },
        bts.stickySwitch{
            triggerID = 5,
            timeDirection = 'forwards',
            attachment = {platform = nil, xOffset = 28.75 * 3200, yOffset = 7.25 * 3200 },
            width = 800,
            height = 1600,
        },
    },
    protoTriggerMods = {
        {
            triggerID = 10,
            triggerClause = "o2 19200 & o3 64000 & ~ t10 |",
        }
    },
}

local SPEED = 120
local H_SPEED = SPEED * 5/8
local triggerID = 13
for i = 7, 14 do
	tempStore.protoCollisions[#tempStore.protoCollisions + 1] = {
		width = 3 * 3200,
		height = 1 * 3200,
		timeDirection = 'forwards',
		lastStateTriggerID = triggerID,
		triggerClause = "t10 19200 =",
		destinations =
		{
			onDestination = {
				xDestination = {
					desiredPosition = (4 + 5*(i - 6)/8) * 3200,
					maxSpeed = H_SPEED,
					acceleration = H_SPEED,
					deceleration = H_SPEED
				},
				yDestination = {
					desiredPosition = i * 3200,
					maxSpeed = SPEED,
					acceleration = SPEED,
					deceleration = SPEED
				}
			},
			offDestination = {
				xDestination = {
					desiredPosition = 4 * 3200,
					maxSpeed = H_SPEED,
					acceleration = H_SPEED,
					deceleration = H_SPEED
				},
				yDestination = {
					desiredPosition = 6 * 3200,
					maxSpeed = SPEED,
					acceleration = SPEED,
					deceleration = SPEED
				}
			}
		}
	}
	triggerID = triggerID + 1
end

for i = 7, 14 do
	tempStore.protoCollisions[#tempStore.protoCollisions + 1] = {
		width = 3 * 3200,
		height = 1 * 3200,
		timeDirection = 'forwards',
		lastStateTriggerID = triggerID,
		triggerClause = "t10 64000 =",
		destinations =
		{
			onDestination = {
				xDestination = {
					desiredPosition = (25 - 5*(i - 6)/8) * 3200,
					maxSpeed = H_SPEED,
					acceleration = H_SPEED,
					deceleration = H_SPEED
				},
				yDestination = {
					desiredPosition = i * 3200,
					maxSpeed = SPEED,
					acceleration = SPEED,
					deceleration = SPEED
				}
			},
			offDestination = {
				xDestination = {
					desiredPosition = 25 * 3200,
					maxSpeed = H_SPEED,
					acceleration = H_SPEED,
					deceleration = H_SPEED
				},
				yDestination = {
					desiredPosition = 6 * 3200,
					maxSpeed = SPEED,
					acceleration = SPEED,
					deceleration = SPEED
				}
			}
		}
	}
	triggerID = triggerID + 1
end

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

getDepartureInformation = bts.getDepartureInformation(tempStore)
