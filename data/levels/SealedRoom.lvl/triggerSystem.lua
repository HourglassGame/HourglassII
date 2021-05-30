local bts = require "global.basicTriggerSystem"

local tempStore = 
{
	protoPortals =
	{
		{
			attachment = {platform = nil, xOffset = 3 * 3200, yOffset = 14 * 3200},
			index = 1,
			width = 2 * 3200,
			height = 2 * 3200,
			collisionOverlap = 50,
			timeDirection = 'forwards',
			triggerClause = "t2 t6 t10 & &",
			destinationIndex = 1,
			xDestination = 0,
			yDestination = 0,
			relativeTime = true,
			timeDestination = -8 * 60,
			illegalDestination = 1,
			fallable = false,
			winner = true
		}
	},
	protoCollisions = {
		{
			width = 17 * 3200,
			height = 1 * 3200,
			timeDirection = 'forwards',
			lastStateTriggerID = 14,
			buttonTriggerID = 13,
			destinations =
			{
				onDestination = {
					xDestination = {
						desiredPosition = 29 * 3200,
						maxSpeed = 960,
						acceleration = 12,
						deceleration = 240
					},
					yDestination = {
						desiredPosition = 10 * 3200,
						maxSpeed = 300,
						acceleration = 60,
						deceleration = 180
					}
				},
				offDestination = {
					xDestination = {
						desiredPosition = 12 * 3200,
						maxSpeed = 200,
						acceleration = 20,
						deceleration = 20
					},
					yDestination = {
						desiredPosition = 10 * 3200,
						maxSpeed = 300,
						acceleration = 60,
						deceleration = 180
					}
				}
			}
		},
	},
	protoMutators = {
	},
	protoButtons = {
		bts.stickyLaserSwitch{
			triggerID = 13,
			timeDirection = 'forwards',
			attachment = {platform = nil, xOffset = 2 * 3200, yOffset = 4.5 * 3200 },
			beamLength = 3*3200,
			beamDirection = 4,
		},
	}
}

local triggerID = 15
for i = 1, 3 do
	for j = 1, 3 do
		tempStore.protoMutators[#tempStore.protoMutators + 1] = bts.pickup{
			attachment = {platform = nil, xOffset = (2.25 + i*0.5) * 3200, yOffset = (11 + j*0.5)  * 3200},
			width = 1600,
			height = 1600,
			timeDirection = 'forwards',
			triggerID = triggerID,
			pickupType = 'timeJump'
		}
		triggerID = triggerID + 1
	end
end
for i = 1, 2 do
	for j = 1, 3 do
		tempStore.protoMutators[#tempStore.protoMutators + 1] = bts.pickup{
			attachment = {platform = nil, xOffset = (3.75 + i*0.5) * 3200, yOffset = (11 + j*0.5)  * 3200},
			width = 1600,
			height = 1600,
			timeDirection = 'forwards',
			triggerID = triggerID,
			pickupType = 'timeGun'
		}
		triggerID = triggerID + 1
	end
end

local portalDestination = {
	[2] = 5,
	[3] = 6,
	[4] = 7,
	[5] = 2,
	[6] = 3,
	[7] = 4,
}
local portalSeconds = {
	[2] = 24,
	[3] = -36,
	[4] = 28,
	[5] = -24,
	[6] = 36,
	[7] = -28,
}

local offsetX = 9 * 3200
local triggerIndex = 1
for i = 1, 3 do
	local portalIndex = #tempStore.protoPortals + 1
	tempStore.protoPortals[portalIndex] = {
		attachment = {platform = nil, xOffset = offsetX + i*6 * 3200, yOffset = 3 * 3200},
		index = portalIndex,
		width = 2 * 3200,
		height = 2 * 3200,
		collisionOverlap = 50,
		timeDirection = 'forwards',
		destinationIndex = portalDestination[portalIndex],
		xDestination = 0,
		yDestination = 0,
		relativeTime = true,
		timeDestination = portalSeconds[portalIndex] * 60,
		illegalDestination = portalDestination[portalIndex],
		fallable = true,
		winner = false
	}
	portalIndex = #tempStore.protoPortals + 1
	tempStore.protoPortals[portalIndex] = {
		attachment = {platform = nil, xOffset = offsetX + i*6 * 3200, yOffset = 14 * 3200},
		index = portalIndex,
		width = 2 * 3200,
		height = 2 * 3200,
		collisionOverlap = 50,
		timeDirection = 'forwards',
		destinationIndex = portalDestination[portalIndex],
		xDestination = 0,
		yDestination = 0,
		relativeTime = true,
		timeDestination = portalSeconds[portalIndex] * 60,
		illegalDestination = portalDestination[portalIndex],
		fallable = true,
		winner = false
	}

	tempStore.protoButtons[#tempStore.protoButtons + 1] = bts.stickyLaserSwitch{
		triggerID = triggerIndex,
		attachment = {platform = nil, xOffset = offsetX + i*6 * 3200, yOffset = 7.5 * 3200 },
		beamLength = 2*3200,
		beamDirection = 4,
		timeDirection = 'forwards',
	}
	tempStore.protoButtons[#tempStore.protoButtons + 1] = bts.momentarySwitch{
		triggerID = triggerIndex + 1,
		attachment = {platform = 1, xOffset = -2.75 * 3200 + i*6 * 3200, yOffset = -0.25 * 3200 },
		width = 4800,
		height = 800,
		pressForceReq = 2,
		timeDirection = 'forwards',
	}

	tempStore.protoCollisions[#tempStore.protoCollisions + 1] = {
		width = 2 * 3200,
		height = 2 * 3200,
		timeDirection = 'forwards',
		lastStateTriggerID = triggerIndex + 2,
		buttonTriggerID = triggerIndex,
		destinations =
		{
			onDestination = {
				xDestination = {
					desiredPosition = offsetX + i*6 * 3200 - 3 * 3200,
					maxSpeed = 200,
					acceleration = 50,
					deceleration = 50
				},
				yDestination = {
					desiredPosition = 8 * 3200,
					maxSpeed = 400,
					acceleration = 60,
					deceleration = 400
				}
			},
			offDestination = {
				xDestination = {
					desiredPosition = offsetX + i*6 * 3200 - 3 * 3200,
					maxSpeed = 200,
					acceleration = 50,
					deceleration = 50
				},
				yDestination = {
					desiredPosition = 6 * 3200,
					maxSpeed = 300,
					acceleration = 60,
					deceleration = 180
				}
			}
		}
	}

	tempStore.protoCollisions[#tempStore.protoCollisions + 1] = {
		width = 0.5 * 3200,
		height = 1 * 3200,
		timeDirection = 'forwards',
		lastStateTriggerID = triggerIndex + 3,
		buttonTriggerID = 13,
		destinations =
		{
			onDestination = {
				xDestination = {
					desiredPosition = offsetX + i*6 * 3200 + 1.5 * 3200,
					maxSpeed = 45,
					acceleration = 10,
					deceleration = 10
				},
				yDestination = {
					desiredPosition = 9 * 3200,
					maxSpeed = 400,
					acceleration = 60,
					deceleration = 400
				}
			},
			offDestination = {
				xDestination = {
					desiredPosition = offsetX + i*6.25 * 3200 + 1.75 * 3200,
					maxSpeed = 200,
					acceleration = 20,
					deceleration = 20
				},
				yDestination = {
					desiredPosition = 9 * 3200,
					maxSpeed = 300,
					acceleration = 60,
					deceleration = 180
				}
			}
		}
	}

	triggerIndex = triggerIndex + 4
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
