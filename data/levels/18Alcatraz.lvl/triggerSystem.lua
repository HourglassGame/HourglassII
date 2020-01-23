local bts = require "global.basicTriggerSystem"

local tempStore =
{
    protoPortals =
    {
        {
            attachment = {platform = 1, xOffset = 0.5 * 3200, yOffset = -2 * 3200},
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
            illegalDestination = nil,
            fallable = false,
            winner = true
        }
    },
    protoCollisions = {
        {
            width = 3 * 3200,
            height = 0.5 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 3,
            buttonTriggerID = 2,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 9.5 * 3200,
                        maxSpeed = 120,
                        acceleration = 15,
                        deceleration = 15
                    },
                    yDestination = {
                        desiredPosition = 13 * 3200,
                        maxSpeed = 120,
                        acceleration = 15,
                        deceleration = 15
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 17.5 * 3200,
                        maxSpeed = 120,
                        acceleration = 15,
                        deceleration = 15
                    },
                    yDestination = {
                        desiredPosition = 5 * 3200,
                        maxSpeed = 120,
                        acceleration = 15,
                        deceleration = 15
                    }
                }
            }
        },
    },
    protoMutators = {
        bts.pickup{
            attachment = {platform = nil, xOffset = 17.25 * 3200, yOffset = 12.25 * 3200},
            width = 1600,
            height = 1600,
            timeDirection = 'forwards',
            triggerID = 1,
            pickupType = 'timeJump'
        },
        bts.spikes{
            attachment = {platform = 1, xOffset = 0 * 3200, yOffset = 0.5 * 3200},
            width = 3 * 3200,
            height = 0.5 * 3200,
            timeDirection = 'forwards',
        },
        bts.spikes{
            attachment = {platform = 1, xOffset = 0 * 3200, yOffset = -3 * 3200},
            width = 3 * 3200,
            height = 0.5 * 3200,
            timeDirection = 'forwards',
        },
        bts.spikes{
            attachment = {platform = 1, xOffset = -0.5 * 3200, yOffset = -2.5 * 3200},
            width = 0.5 * 3200,
            height = 3 * 3200,
            timeDirection = 'forwards',
        },
        bts.spikes{
            attachment = {platform = 1, xOffset = 3 * 3200, yOffset = -2.5 * 3200},
            width = 0.5 * 3200,
            height = 3 * 3200,
            timeDirection = 'forwards',
        },
    },
    protoButtons = {
    },
    protoTriggerMods = {
        {
            triggerID = 2,
            triggerClause = "t2 !",
        }
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

local btsGetDeparture = bts.getDepartureInformation(tempStore)
function getDepartureInformation(departures)
    btsGetDeparture(departures)
    
    tempStore.outputTriggers[2] = {1 - tempStore.triggerArrivals[2][1]}
    
    return
        tempStore.outputTriggers,
        tempStore.forwardsGlitz,
        tempStore.reverseGlitz,
        tempStore.persistentGlitz,
        tempStore.additionalEndBoxes
end