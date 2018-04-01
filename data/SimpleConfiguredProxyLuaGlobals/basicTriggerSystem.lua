local function cloneAttachment(a)
    return {platform = a.platform, xOffset = a.xOffset, yOffset = a.yOffset}
end

local function momentarySwitch(p)
    return {
        type = 'momentarySwitch',
        timeDirection = p.timeDirection,
        attachment = cloneAttachment(p.attachment),
        width = p.width,
        height = p.height,
        triggerID = p.triggerID,
        stateTriggerID = p.stateTriggerID,
        extraTriggerIDs = p.extraTriggerIDs,
    }
end

local function stickySwitch(p)
    return {
        type = 'stickySwitch',
        timeDirection = p.timeDirection,
        attachment = cloneAttachment(p.attachment),
        width = p.width,
        height = p.height,
        triggerID = p.triggerID,
        stateTriggerID = p.stateTriggerID,
        extraTriggerIDs = p.extraTriggerIDs,
    }
end

local function pickup(p)
    return {
        type = 'pickup',
        timeDirection = p.timeDirection,
        attachment = cloneAttachment(p.attachment),
        width = p.width,
        height = p.height,
        pickupType = p.pickupType,
        triggerID = p.triggerID,
        pickupNumber = p.pickupNumber,
    }
end
local function spikes(p)
    return {
        type = 'spikes',
        timeDirection = p.timeDirection,
        attachment = cloneAttachment(p.attachment),
        width = p.width,
        height = p.height,
    }
end
local function wireGlitz(p)
end
local function easyWireGlitz(p)
end
local function basicRectangleGlitz(p)
end
local function basicTextGlitz(p)
end
local function calculatePhysicsAffectingStuff(tempStore)
    return tempStore
end
local function getDepartureInformation(tempStore)
    return tempStore
end
local function mutateObject(tempStore)
    return tempStore
end
return {
    momentarySwitch = momentarySwitch,
    --toggleSwitch = toggleSwitch,
    stickySwitch = stickySwitch,
    --stickyLaserSwitch = stickyLaserSwitch,
    --multiStickySwitch = multiStickySwitch,
    pickup = pickup,
    spikes = spikes,
    --boxOMatic = boxOMatic,
    wireGlitz = wireGlitz,
    easyWireGlitz = easyWireGlitz,
    basicRectangleGlitz = basicRectangleGlitz,
    basicTextGlitz = basicTextGlitz,
    mutateObject = mutateObject,
    calculatePhysicsAffectingStuff = calculatePhysicsAffectingStuff,
    getDepartureInformation = getDepartureInformation,
    --calculateBidirectionalGlitz = calculateBidirectionalGlitz,
    --timeDirectionToInt = timeDirectionToInt,
}
