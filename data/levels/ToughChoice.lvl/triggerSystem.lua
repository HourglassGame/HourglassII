local bts = require "global.basicTriggerSystem"

local tempStore =
{
	protoPortals =
	{
		{
			attachment = {platform = 6, xOffset = 1 * 3200, yOffset = -2 * 3200},
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
		{
			attachment = {platform = nil, xOffset = 17 * 3200, yOffset = 11 * 3200},
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
			winner = false
		},
	},
	protoCollisions = {
		{
			width = 1 * 3200,
			height = 1 * 3200,
			timeDirection = 'forwards',
			lastStateTriggerID = 2,
			buttonTriggerID = 1,
			destinations =
			{
				onDestination = {
					xDestination = {
						desiredPosition = 1 * 3200,
						maxSpeed = 200,
						acceleration = 50,
						deceleration = 50
					},
					yDestination = {
						desiredPosition = 1 * 3200,
						maxSpeed = 5000,
						acceleration = 5000,
						deceleration = 5000
					}
				},
				offDestination = {
					xDestination = {
						desiredPosition = 26 * 3200,
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
				}
			}
		},
		{
			width = 1 * 3200,
			height = 1 * 3200,
			timeDirection = 'forwards',
			lastStateTriggerID = 3,
			triggerClause = "t10 ! t11 ! |",
			destinations =
			{
				onDestination = {
					xDestination = {
						desiredPosition = 1 * 3200,
						maxSpeed = 200,
						acceleration = 50,
						deceleration = 50
					},
					yDestination = {
						desiredPosition = 2.5 * 3200,
						maxSpeed = 8000,
						acceleration = 8000,
						deceleration = 8000
					}
				},
				offDestination = {
					xDestination = {
						desiredPosition = 1 * 3200,
						maxSpeed = 200,
						acceleration = 50,
						deceleration = 50
					},
					yDestination = {
						desiredPosition = 1 * 3200,
						maxSpeed = 5000,
						acceleration = 5000,
						deceleration = 5000
					}
				}
			}
		},
		{
			width = 1 * 3200,
			height = 1 * 3200,
			timeDirection = 'forwards',
			lastStateTriggerID = 4,
			triggerClause = "t9 ! t11 ! |",
			destinations =
			{
				onDestination = {
					xDestination = {
						desiredPosition = 1 * 3200,
						maxSpeed = 200,
						acceleration = 50,
						deceleration = 50
					},
					yDestination = {
						desiredPosition = 2.5 * 3200,
						maxSpeed = 8000,
						acceleration = 8000,
						deceleration = 8000
					}
				},
				offDestination = {
					xDestination = {
						desiredPosition = 1 * 3200,
						maxSpeed = 200,
						acceleration = 50,
						deceleration = 50
					},
					yDestination = {
						desiredPosition = 1 * 3200,
						maxSpeed = 5000,
						acceleration = 5000,
						deceleration = 5000
					}
				}
			}
		},
		{
			width = 1 * 3200,
			height = 1 * 3200,
			timeDirection = 'forwards',
			lastStateTriggerID = 5,
			triggerClause = "t9 ! t10 ! |",
			destinations =
			{
				onDestination = {
					xDestination = {
						desiredPosition = 1 * 3200,
						maxSpeed = 200,
						acceleration = 50,
						deceleration = 50
					},
					yDestination = {
						desiredPosition = 2.5 * 3200,
						maxSpeed = 8000,
						acceleration = 8000,
						deceleration = 8000
					}
				},
				offDestination = {
					xDestination = {
						desiredPosition = 1 * 3200,
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
				}
			}
		},
		{
			width = 1 * 3200,
			height = 2 * 3200,
			timeDirection = 'forwards',
			lastStateTriggerID = 6,
			triggerClause = "t9 t10 & t11 & !",
			destinations =
			{
				onDestination = {
					xDestination = {
						desiredPosition = 7.5 * 3200,
						maxSpeed = 200,
						acceleration = 50,
						deceleration = 50
					},
					yDestination = {
						desiredPosition = 11 * 3200,
						maxSpeed = 200,
						acceleration = 50,
						deceleration = 50
					}
				},
				offDestination = {
					xDestination = {
						desiredPosition = 7.5 * 3200,
						maxSpeed = 200,
						acceleration = 50,
						deceleration = 50
					},
					yDestination = {
						desiredPosition = 13 * 3200,
						maxSpeed = 300,
						acceleration = 50,
						deceleration = 50
					}
				}
			}
		},
		{
			width = 4 * 3200,
			height = 1 * 3200,
			timeDirection = 'forwards',
			lastStateTriggerID = 13,
			buttonTriggerID = 14,
			destinations =
			{
				onDestination = {
					xDestination = {
						desiredPosition = 11 * 3200,
						maxSpeed = 200,
						acceleration = 50,
						deceleration = 50
					},
					yDestination = {
						desiredPosition = 10 * 3200,
						maxSpeed = 200,
						acceleration = 18,
						deceleration = 18
					}
				},
				offDestination = {
					xDestination = {
						desiredPosition = 11 * 3200,
						maxSpeed = 200,
						acceleration = 50,
						deceleration = 50
					},
					yDestination = {
						desiredPosition = 5 * 3200,
						maxSpeed = 200,
						acceleration = 18,
						deceleration = 18
					}
				}
			}
		},
		{
			width = 1 * 3200,
			height = 11 * 3200,
			timeDirection = 'forwards',
			lastStateTriggerID = 15,
			buttonTriggerID = 1,
			destinations =
			{
				onDestination = {
					xDestination = {
						desiredPosition = 24.5 * 3200,
						maxSpeed = 200,
						acceleration = 50,
						deceleration = 50
					},
					yDestination = {
						desiredPosition = -5 * 3200,
						maxSpeed = 200,
						acceleration = 50,
						deceleration = 50
					}
				},
				offDestination = {
					xDestination = {
						desiredPosition = 24.5 * 3200,
						maxSpeed = 200,
						acceleration = 50,
						deceleration = 50
					},
					yDestination = {
						desiredPosition = 6 * 3200,
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
			attachment = {platform = nil, xOffset = 28.75 * 3200, yOffset = 12.75 * 3200},
			width = 1600,
			height = 1600,
			timeDirection = 'forwards',
			triggerID = 7,
			pickupType = 'timeJump'
		},
		bts.pickup{
			attachment = {platform = nil, xOffset = 21.75 * 3200, yOffset = 10.25 * 3200},
			width = 1600,
			height = 1600,
			timeDirection = 'forwards',
			triggerID = 8,
			pickupType = 'timeReverse'
		},
		bts.pickup{
			attachment = {platform = nil, xOffset = 2.5 * 3200, yOffset = 12.5 * 3200},
			width = 1600,
			height = 1600,
			timeDirection = 'forwards',
			triggerID = 9,
			pickupType = 'timeJump'
		},
		bts.pickup{
			attachment = {platform = nil, xOffset = 4.25 * 3200, yOffset = 12.5 * 3200},
			width = 1600,
			height = 1600,
			timeDirection = 'forwards',
			triggerID = 10,
			pickupType = 'timeReverse'
		},
		bts.pickup{
			attachment = {platform = nil, xOffset = 6 * 3200, yOffset = 12.5 * 3200},
			width = 1600,
			height = 1600,
			timeDirection = 'forwards',
			triggerID = 11,
			pickupType = 'timeGun'
		},
		bts.spikes{
			attachment = {platform = 1, xOffset = -0.5 * 3200, yOffset = 5 * 3200},
			width = 0.5 * 3200,
			height = 12 * 3200,
			timeDirection = 'forwards',
		},
		bts.spikes{
			attachment = {platform = 2, xOffset = 1 * 3200, yOffset = 9 * 3200},
			width = 1.5 * 3200,
			height = 2 * 3200,
			timeDirection = 'forwards',
		},
		bts.spikes{
			attachment = {platform = 3, xOffset = 2.75 * 3200, yOffset = 9 * 3200},
			width = 1.5 * 3200,
			height = 2 * 3200,
			timeDirection = 'forwards',
		},
		bts.spikes{
			attachment = {platform = 4, xOffset = 4.5 * 3200, yOffset = 9 * 3200},
			width = 1.5 * 3200,
			height = 2 * 3200,
			timeDirection = 'forwards',
		},
	},
	protoButtons = {
		bts.multiStickySwitch{
			buttons = {
				{
					attachment = {platform = nil, xOffset = 16.25 * 3200, yOffset = 16.75 * 3200},
					width = 1600,
					height = 800,
				},
				{
					attachment = {platform = nil, xOffset = 18.75 * 3200, yOffset = 16.75 * 3200},
					width = 1600,
					height = 800,
				},
				{
					attachment = {platform = nil, xOffset = 21.25 * 3200, yOffset = 16.75 * 3200},
					width = 1600,
					height = 800,
				},
			},
			timeDirection = 'forwards',
			triggerID = 1,
			stateTriggerID = 12
		},
		bts.momentarySwitch{
			attachment = {platform = nil, xOffset = 28.5 * 3200, yOffset = 14.75 * 3200},
			width = 3200,
			height = 800,
			timeDirection = 'forwards',
			triggerID = 14
		}
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
