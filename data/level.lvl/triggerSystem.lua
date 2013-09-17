local bts = require "global.basicTriggerSystem"

local tempStore = 
{
    protoPortals =
    {
        {
            attachment = {platform = 1, xOffset = -4200, yOffset = -3200},
            index = 1,
            width = 4200,
            height = 4200,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = 1,
            xDestination = 0,
            yDestination = -16000,
            relativeTime = true,
            timeDestination = 120,
            illegalDestination = 1,
            fallable = true,
            winner = false,
        }
    },
    protoCollisions = {
        {
            width = 6400,
            height = 1600,
            timeDirection = 'forwards',
            lastStateTriggerID = 2,
            buttonTriggerID = 1,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 22400,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 43800,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 38400,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 43800,
                        maxSpeed = 300,
                        acceleration = 20,
                        deceleration = 20
                    }
                }
            }
        }
    },
    protoMutators = {},
    protoMuts = {
        {
            data = {
                x = 50000,
                y = 25600,
                xspeed = 0,
                yspeed = 0,
                width = 800,
                height = 6400,
                collisionOverlap = 0,
                timeDirection = "forwards",
            },
            effect = function (self, object) -- butterfingers
                if object.type == "guy" and object.boxCarrying then
                    object.boxCarrying = false
                    self.additionalEndBoxes[#self.additionalEndBoxes+1] = {
                        box = {
                            x = object.x + object.width/2 - object.boxCarrySize/2, 
                            y = object.y - object.boxCarrySize, 
                            xspeed = -object.xspeed, yspeed = -500, 
                            size = object.boxCarrySize, 
                            illegalPortal = nil, 
                            arrivalBasis = nil, 
                            timeDirection = object.boxCarryDirection}, 
                        targetFrame = self.frameNumber+bts.timeDirectionToInt(object.boxCarryDirection),
                    }
                end
                return object
            end,
        }
    },
    protoButtons = {
        bts.momentarySwitch{
            attachment = {platform = 1, xOffset = 3200, yOffset = -800},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 1
        },
        bts.momentarySwitch{
            attachment = {platform = nil, xOffset = 3200, yOffset = 37600},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 3
        }
    }
}
local function calculateMutatorGlitz(protoMutator)
    return bts.calculateBidirectionalGlitz(450, protoMutator, {r = 150, g = 150, b = 150}, {r = 150, g = 150, b = 150})
end
--==Callin Definitions==--
--triggerArrivals have already had default values inserted by C++
--for trigger indices that did not arrive by the time this is called
function calculatePhysicsAffectingStuff(frameNumber, triggerArrivals)
    local retv = bts.calculatePhysicsAffectingStuff(tempStore)(frameNumber, triggerArrivals)
    
    tempStore.makeBox = (frameNumber == 2000 and triggerArrivals[3][1] == 0)
    retv.mutators = { [1] = tempStore.protoMuts[1].data}
    tempStore.portalActive = (triggerArrivals[3][1] == 1)
    
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
    return tempStore.portalActive
end
function mutateObject(responsibleManipulatorIndices, dynamicObject)
    return tempStore.protoMuts[1].effect(tempStore, dynamicObject)
end

function getDepartureInformation(departures)
    local outputTriggers, forwardsGlitz, reverseGlitz, additionalEndBoxes = bts.getDepartureInformation(tempStore)(departures)
    
    local forwardsMutGlitz, reverseMutGlitz = calculateMutatorGlitz(tempStore.protoMuts[1].data)
    table.insert(forwardsGlitz, forwardsMutGlitz)
    table.insert(reverseGlitz, reverseMutGlitz)

    if tempStore.makeBox then
        additionalEndBoxes[#additionalEndBoxes+1] = {
            box = {x = 12800, y = 6400, xspeed = -600, yspeed = -400, size = 3200,
                   illegalPortal = nil, arrivalBasis = nil, timeDirection = 'forwards'}, 
            targetFrame = 500
        }
    end
    
    return outputTriggers, forwardsGlitz, reverseGlitz, additionalEndBoxes
end