local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 27.5 * 3200, yOffset = 9 * 3200},
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
            attachment = {platform = nil, xOffset = 2.5 * 3200, yOffset = 9 * 3200},
            index = 2,
            width = 2 * 3200,
            height = 2 * 3200,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = 2,
            xDestination = 0,
            yDestination = 0,
            relativeTime = false,
            timeDestination = 0 * 60,
            illegalDestination = 2,
            fallable = false,
            winner = false
        }
    },
    protoCollisions = {
        {
            width = 1 * 3200,
            height = 3 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 3,
            buttonTriggerID = 2,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 25 * 3200,
                        maxSpeed = 250,
                        acceleration = 20,
                        deceleration = 20
                    },
                    yDestination = {
                        desiredPosition = 5 * 3200,
                        maxSpeed = 250,
                        acceleration = 20,
                        deceleration = 20
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 25 * 3200,
                        maxSpeed = 250,
                        acceleration = 20,
                        deceleration = 20
                    },
                    yDestination = {
                        desiredPosition = 8 * 3200,
                        maxSpeed = 250,
                        acceleration = 20,
                        deceleration = 20
                    }
                }
            }
        },
        {
            width = 2 * 3200,
            height = 2.5 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 4,
            buttonTriggerID = 1,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 20 * 3200,
                        maxSpeed = 160,
                        acceleration = 20,
                        deceleration = 20
                    },
                    yDestination = {
                        desiredPosition = 5 * 3200,
                        maxSpeed = 160,
                        acceleration = 20,
                        deceleration = 20
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 7 * 3200,
                        maxSpeed = 160,
                        acceleration = 20,
                        deceleration = 20
                    },
                    yDestination = {
                        desiredPosition = 5 * 3200,
                        maxSpeed = 160,
                        acceleration = 20,
                        deceleration = 20
                    }
                }
            }
        },
        {
            width = 2 * 3200,
            height = 1 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 5,
            buttonTriggerID = 1,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 20 * 3200,
                        maxSpeed = 160,
                        acceleration = 20,
                        deceleration = 20
                    },
                    yDestination = {
                        desiredPosition = 14 * 3200,
                        maxSpeed = 160,
                        acceleration = 20,
                        deceleration = 20
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 7 * 3200,
                        maxSpeed = 160,
                        acceleration = 20,
                        deceleration = 20
                    },
                    yDestination = {
                        desiredPosition = 14 * 3200,
                        maxSpeed = 160,
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
        bts.stickySwitch{
            attachment = {platform = nil, xOffset = 7 * 3200 - 800, yOffset = 13.75 * 3200},
            width = 800,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 1
        },
        bts.momentarySwitch{
            attachment = {platform = nil, xOffset = 20.5 * 3200, yOffset = 17 * 3200 - 800},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 2
        }
    },
    protoGlitz = {
        bts.basicRectangleGlitz{
            x = 8 * 3200,
            y = 14.5 * 3200,
            width = 11 * 3200,
            height = 2.5 * 3200,
            layer = 1600,
            colour = {r = 150, g = 90, b = 45},
        },
        bts.basicTextGlitz{
            x = 8.25 * 3200,
            y = 14.65 * 3200,
            text = "Pause (keyboard P) not just for taking a",
            size = 1600,
            layer = 1600,
            colour = {r = 0, g = 0, b = 0},
        },
        bts.basicTextGlitz{
            x = 8.25 * 3200,
            y = (14.65 + 0.48) * 3200,
            text = "break, as actions may be performed while",
            size = 1600,
            layer = 1600,
            colour = {r = 0, g = 0, b = 0},
        },
        bts.basicTextGlitz{
            x = 8.25 * 3200,
            y = (14.65 + 2*0.48) * 3200,
            text = "paused. Non-movement actions automatically",
            size = 1600,
            layer = 1600,
            colour = {r = 0, g = 0, b = 0},
        },
        bts.basicTextGlitz{
            x = 8.25 * 3200,
            y = (14.65 + 3*0.48) * 3200,
            text = "advance time by 1/30th of a second.",
            size = 1600,
            layer = 1600,
            colour = {r = 0, g = 0, b = 0},
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

getDepartureInformation = bts.getDepartureInformation(tempStore)
