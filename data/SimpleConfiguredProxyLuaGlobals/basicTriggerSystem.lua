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
local function cloneButtonSegment(bs)
    return {attachment = cloneAttachment(bs.attachment), width = bs.width, height = bs.height}
end
local function toggleSwitch(p)
    return {
        type = 'toggleSwitch',
        timeDirection = p.timeDirection,
        first = cloneButtonSegment(p.first),
        second = cloneButtonSegment(p.second),
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
local function stickyLaserSwitch(p)
    return {
        type = 'stickyLaserSwitch',
        timeDirection = p.timeDirection,
        attachment = cloneAttachment(p.attachment),
        beamLength = p.beamLength,
        beamDirection = p.beamDirection,
        triggerID = p.triggerID,
        stateTriggerID = p.stateTriggerID,
        extraTriggerIDs = p.extraTriggerIDs,
    }
end
local function multiStickySwitch(p)
    local buttons = {}
    for i = 1, #p.buttons do
        buttons[i] = cloneButtonSegment(p.buttons[i])
    end
    return {
        type = 'multiStickySwitch',
        timeDirection = p.timeDirection,
        buttons = buttons,
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
local function clonePlatformAndPos(p)
    return {
        platform = p.platform,
        pos = p.pos,
    }
end
local function wireGlitz(p)
    return {
        type = 'wireGlitz',
        triggerID = p.triggerID,
        useTriggerArrival = p.useTriggerArrival,
        x1 = clonePlatformAndPos(p.x1),
        y1 = clonePlatformAndPos(p.y1),
        x2 = clonePlatformAndPos(p.x2),
        y2 = clonePlatformAndPos(p.y2),
    }
end
local function easyWireGlitz(p)
    if p.x then
        p.x1 = {platform = p.x.platform, pos = p.x.pos - 0.05 * 3200}
        p.x2 = {platform = p.x.platform, pos = p.x.pos + 0.05 * 3200}
        p.x = nil
    end
    if p.y then
        p.y1 = {platform = p.y.platform, pos = p.y.pos - 0.05 * 3200}
        p.y2 = {platform = p.y.platform, pos = p.y.pos + 0.05 * 3200}
        p.y = nil
    end
    return wireGlitz(p)
end
local function basicRectangleGlitz(p)
    return {
        type = 'basicRectangleGlitz',
        colour = p.colour,
        layer = p.layer,
        x = p.x,
        y = p.y,
        width = p.width,
        height = p.height,
    }
end
local function basicTextGlitz(p)
    return {
        type = 'basicTextGlitz',
        x = p.x,
        y = p.y,
        text = p.text,
        size = p.size,
        layer = p.layer,
        colour = p.colour,
    }
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
    toggleSwitch = toggleSwitch,
    stickySwitch = stickySwitch,
    stickyLaserSwitch = stickyLaserSwitch,
    multiStickySwitch = multiStickySwitch,
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
