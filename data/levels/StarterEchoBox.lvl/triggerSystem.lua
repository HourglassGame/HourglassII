local bts = require "global.basicTriggerSystem"

local tempStore = 
{
    protoPortals =
    {
        {
            attachment = {xOffset = 12000, yOffset = 40600},
            index = 1,
            width = 4200,
            height = 4200,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = 1,
            xDestination = 0,
            yDestination = 0,
            relativeTime = true,
            timeDestination = 30,
            illegalDestination = nil,
            fallable = true,
            winner = false,
        },
        {
            attachment = {xOffset = 44500, yOffset = 31000},
            index = 2,
            width = 4200,
            height = 4200,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = 2,
            xDestination = 0,
            yDestination = 0,
            relativeTime = false,
            timeDestination = 0,
            illegalDestination = nil,
            fallable = false,
            winner = false,
        },
        {
            attachment = {xOffset = 52800, yOffset = 28800},
            index = 3,
            width = 6400,
            height = 6400,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = nil,
            xDestination = 0,
            yDestination = 0,
            relativeTime = false,
            timeDestination = 0,
            illegalDestination = nil,
            fallable = true,
            winner = true,
        }
    },
    protoCollisions = {
    },
    protoMutators = {  
    },
    protoButtons = {
    }
}
--[[
local function calculateMutatorGlitz(protoMutator)
    return bts.calculateBidirectionalGlitz(450, protoMutator, {r = 150, g = 150, b = 150}, {r = 150, g = 150, b = 150})
end
]]--
--==Callin Definitions==--
--triggerArrivals have already had default values inserted by C++
--for trigger indices that did not arrive by the time this is called
--[[
function calculatePhysicsAffectingStuff(frameNumber, triggerArrivals)
    local retv = bts.calculatePhysicsAffectingStuff(tempStore)(frameNumber, triggerArrivals)
    
    return retv
end
]]--

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
--[[
function mutateObject(responsibleManipulatorIndices, dynamicObject)
    return tempStore.protoMutators[1].effect(tempStore, dynamicObject)
end
]]--
getDepartureInformation = bts.getDepartureInformation(tempStore)
--[[
function getDepartureInformation(departures)
    local outputTriggers, forwardsGlitz, reverseGlitz, additionalEndBoxes = bts.getDepartureInformation(tempStore)(departures)
    
    local forwardsMutGlitz, reverseMutGlitz = forwardsMutGlitz, reverseMutGlitz
    
    return outputTriggers, forwardsGlitz, reverseGlitz, additionalEndBoxes
end
]]--
