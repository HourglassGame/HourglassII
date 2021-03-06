local framesPerSecond = 60

local function map(f, l)
    local r = {}
    for i, v in ipairs(l) do
        r[i] = f(v)
    end
    return r
end

local function formatTime(frames)
    local withDecimal = string.format("%.1f", frames/framesPerSecond)
    local withoutDecimal = math.floor(0.5+frames/framesPerSecond)
    if withDecimal == withoutDecimal .. ".0" then
        return withoutDecimal .. "s"
    end
    return withDecimal.."s"
end

local function calculateBidirectionalGlitz(layer, obj, forwardsColour, reverseColour)
    local sameDirectionGlitz = {
        layer = layer,
        type = "rectangle",
        x = obj.x - (obj.xspeed or 0), y = obj.y - (obj.yspeed or 0),
        width = obj.width, height = obj.height,
        colour = forwardsColour
    }
    local oppositeDirectionGlitz = {
        layer = layer,
        type = "rectangle",
        x = obj.x - (obj.xspeed or 0), y = obj.y - (obj.yspeed or 0),
        width = obj.width, height = obj.height,
        colour = reverseColour
    }
    if obj.timeDirection == 'forwards' then
        return sameDirectionGlitz, oppositeDirectionGlitz
    else
        return oppositeDirectionGlitz, sameDirectionGlitz
    end
end

local function calculateCollisions(protoCollisions, triggerArrivals, outputTriggers, frameNumber)
    local function calculateCollision(self, triggerArrivals, outputTriggers)
        if self.rawCollisionFunction then
            return self.rawCollisionFunction(triggerArrivals, outputTriggers, frameNumber)
        end
        
        local function solvePDEquation(destination, position, velocity)
            local function square(a) return a * a end
            local function sign(a) return math.abs(a) / a end
            
            local desiredPosition = destination.desiredPosition
            local deceleration = destination.deceleration
            local acceleration = destination.acceleration
            if desiredPosition ~= position then
                if 
                    (math.abs(desiredPosition - position) <= math.abs(velocity)) 
                        and
                    (math.abs(velocity) <= deceleration)
                then
                    velocity = desiredPosition - position
                else
                    local direction = sign(position - desiredPosition)
                    if velocity * direction > 0 then
                        velocity = velocity - direction * deceleration
                        if velocity * direction < 0 then
                            velocity = 0
                        end
                    elseif
                        math.abs(position - desiredPosition)
                            >
                        (square(velocity - direction * acceleration) * 3 / (2 * deceleration))
                    then
                        velocity = velocity - direction * acceleration
                    elseif math.abs(position - desiredPosition) > (square(velocity)*3/(2*deceleration))
                    then
                        velocity =
                            -direction * math.sqrt(
                                math.abs(position - desiredPosition)
                                * deceleration * 2 / 3)
                    else
                        velocity = velocity + direction * deceleration
                    end
                end
            else
                if math.abs(velocity) <= deceleration then
                    velocity = 0
                else
                    velocity = velocity + sign(velocity)*deceleration
               end
            end
            local maxSpeed = destination.maxSpeed
            if math.abs(velocity) > maxSpeed then
                velocity = sign(velocity) * maxSpeed
            end
            velocity = velocity >= 0 and math.floor(velocity) or math.ceil(velocity)
            position = position + velocity
            return {position = position, velocity = velocity}
        end
        
        
        local active = (self.buttonTriggerID and triggerArrivals[self.buttonTriggerID][1] > 0) 
                    or (self.triggerFunction and self.triggerFunction(triggerArrivals, frameNumber))
        local destination = active and self.destinations.onDestination or self.destinations.offDestination

        local lastStateTrigger = triggerArrivals[self.lastStateTriggerID]
        
        local horisontalPosAndVel = solvePDEquation(destination.xDestination, lastStateTrigger[1], lastStateTrigger[3])
        local verticalPosAndVel = solvePDEquation(destination.yDestination, lastStateTrigger[2], lastStateTrigger[4])
        
        return
        {
            x = horisontalPosAndVel.position,
            y = verticalPosAndVel.position,
            xspeed = horisontalPosAndVel.velocity,
            yspeed = verticalPosAndVel.velocity,
            prevXspeed = lastStateTrigger[3],
            prevYspeed = lastStateTrigger[4],
            width = self.width,
            height = self.height,
            timeDirection = self.timeDirection
        }
    end
    return map(function(protoCollision) 
            return calculateCollision(protoCollision, triggerArrivals, outputTriggers) 
        end, protoCollisions)
end


local function snapAttachment(objectTimeDirection, attachment, collisions)
    local x
    local y
    local xspeed
    local yspeed
    if attachment.platform ~= nil then
        local collision = collisions[attachment.platform]
        if collision.timeDirection == objectTimeDirection then
            x = collision.x + attachment.xOffset
            y = collision.y + attachment.yOffset
            xspeed = collision.xspeed
            yspeed = collision.yspeed
        else
            x = collision.x - collision.xspeed + attachment.xOffset
            y = collision.y - collision.yspeed + attachment.yOffset
            xspeed = -collision.xspeed
            yspeed = -collision.yspeed
        end
    else
        x = attachment.xOffset
        y = attachment.yOffset
        xspeed = 0
        yspeed = 0
    end
    return x, y, xspeed, yspeed
end

local function calculatePortals(forwardsGlitz, reverseGlitz, protoPortals, collisions, triggerArrivals, frameNumber)
    local function calculatePortal(protoPortal, collisions)
        local x, y, xspeed, yspeed =
            snapAttachment(protoPortal.timeDirection, protoPortal.attachment, collisions)
        
        local retPortal =  {
            index = protoPortal.index,
            x = x,
            y = y,
            xaim = protoPortal.xaim,
            yaim = protoPortal.yaim,
            width = protoPortal.width,
            height = protoPortal.height,
            xspeed = xspeed,
            yspeed = yspeed,
            collisionOverlap = protoPortal.collisionOverlap,
            timeDirection = protoPortal.timeDirection,
            destinationIndex = protoPortal.destinationIndex,
            xDestination = protoPortal.xDestination,
            yDestination = protoPortal.yDestination,
            relativeTime = protoPortal.relativeTime,
            timeDestination = protoPortal.timeDestination,
            relativeDirection = (protoPortal.relativeDirection ~= nil) or true,
            destinationDirection = protoPortal.destinationDirection or 'forwards',
            illegalDestination = protoPortal.illegalDestination,
            fallable = protoPortal.fallable,
            isLaser = (protoPortal.isLaser or false),
            winner = false
        }
        
        if protoPortal.winner then
            -- Not necessary
            retPortal.winner = true
            retPortal.relativeTime = false
            retPortal.timeDestination = 1
            retPortal.relativeDirection = false
            retPortal.destinationDirection = 'reverse'
        end
        
        local active = true
        if protoPortal.triggerFunction then
            active = protoPortal.triggerFunction(triggerArrivals, frameNumber)
        end
        
        return retPortal, active
    end

    local function calculatePortalGlitz(portal, forwardsGlitz, reverseGlitz, active)
    
        local colour
        if active then 
            colour = {r = 120, g = 120, b = 120}, {r = 120, g = 120, b = 120}
        else
            colour = {r = 70, g = 70, b = 70}, {r = 70, g = 70, b = 70}
        end

        local forGlitz, revGlitz = calculateBidirectionalGlitz(350, portal, colour, colour)
        
        -- Text does not support changing time directions
        local text
        if portal.winner then
            text = active and "Win" or "Inactive"
        elseif portal.relativeTime and portal.timeDestination > 0 then
            text = "+" .. formatTime(portal.timeDestination)
        else
            text = formatTime(portal.timeDestination)
        end
        
        local textGlitz = {
            type = "text",
            x = portal.x+portal.width/2-1600,
            y = portal.y-2400,
            text = text,
            size = 2000,
            layer = 440,
            colour = {r = 0, g = 0, b = 0},
        }
        
        table.insert(forwardsGlitz, forGlitz)
        table.insert(reverseGlitz, revGlitz)
        table.insert(forwardsGlitz, textGlitz)
        table.insert(reverseGlitz, textGlitz)
    end
    
    local function calculateArrivalLocation(portal)
        return
        {
            x = portal.x,
            y = portal.y,
            xspeed = portal.xspeed,
            yspeed = portal.yspeed,
            timeDirection = portal.timeDirection
        }
    end
    
    local portals = {}
    local portalCount = 0
    local arrivalLocations = {}

    for i, protoPortal in ipairs(protoPortals) do
        local portal, active = calculatePortal(protoPortal, collisions)
        if not protoPortal.isLaser then
            calculatePortalGlitz(portal, forwardsGlitz, reverseGlitz, active)
        end
        arrivalLocations[#arrivalLocations + 1] = calculateArrivalLocation(portal)
        if active then
            portalCount = portalCount + 1
            portals[portalCount] = portal
        end
    end
    --[[
    if protoLasers then
        for i, protoLaser in ipairs(protoLasers) do
            local portal, active = calculatePortal(protoLaser, collisions, true)
            arrivalLocations[#arrivalLocations + 1] = calculateArrivalLocation(portal)
            if active then
                portalCount = portalCount + 1
                portals[portalCount] = portal
            end
        end
    end
    ]]--
    return portals, arrivalLocations
end

local function calculateButtonPositionsAndVelocities(protoButtons, collisions)
    for i, protoButton in ipairs(protoButtons) do
        protoButton:calcPnV(collisions)
    end
end

local function fillCollisionTriggers(triggers, protoCollisions, collisions)
    for i = 1, #protoCollisions do
        triggers[protoCollisions[i].lastStateTriggerID] =
            {collisions[i].x, collisions[i].y, collisions[i].xspeed, collisions[i].yspeed}
    end
end


local function list_iter(t)
    local i = 0
    local n = #t
    return function ()
        i = i + 1
        if i <= n then return t[i] end
    end
end

local function calculateCollisionGlitz(collision)
    return calculateBidirectionalGlitz(300, collision, {r = 50, g = 0, b = 0}, {r = 0, g = 0, b = 50})
end

local function calculateButtonStates(protoButtons, departures, triggerArrivals)
    for button in list_iter(protoButtons) do
        button:updateState(departures, triggerArrivals)
    end
end

local function cloneAttachment(a)
    return {platform = a.platform, xOffset = a.xOffset, yOffset = a.yOffset}
end

local function calculateButtonPositionAndVelocity(protoButton, collisions)
    local x, y, xspeed, yspeed =
        snapAttachment(protoButton.timeDirection, protoButton.attachment, collisions)
    return {
        x = x,
        y = y,
        xspeed = xspeed,
        yspeed = yspeed
    }
end

local function temporalIntersectingExclusive(a, b)
    local xa, ya = a.x, a.y
    --if a.timeDirection == 'forwards' then
    --    xa = a.x
    --    ya = a.y
    --else
    --    xa = a.x - a.xspeed
    --    ya = a.y - a.yspeed
    --end
    local wa, ha = a.width, a.height
    local xb, yb = b.x, b.y
    --if b.timeDirection == 'forwards' then
    --    xb = b.x
    --    yb = b.y
    --else
    --    xb = b.x - b.xspeed
    --    yb = b.y - b.yspeed
    --end
    local wb, hb = b.width, b.height
    return
        (
            (xa < xb and xa + wa > xb)
             or
            (xb < xa and xb + wb > xa)
             or
            (xa == xb)
        )
         and
        (
            (ya < yb and ya + ha > yb)
             or
            (yb < ya and yb + hb > ya)
             or
            (ya == yb)
        )
end


local function checkPressed(buttonArea, departures)
    for frame, objectList in pairs(departures) do
        for box in list_iter(objectList.boxes) do
            if temporalIntersectingExclusive(buttonArea, box) then
                return true
            end
        end
        for guy in list_iter(objectList.guys) do
            if temporalIntersectingExclusive(buttonArea, guy) then
                return true
            end
        end
    end
    return false
end

local function constructDynamicArea(proto, PnV)
    return {
        timeDirection = proto.timeDirection,
        width = proto.width, height = proto.height,
        x = PnV.x, y = PnV.y, xspeed = PnV.xspeed, yspeed = PnV.yspeed}
end


local function calculateButtonGlitz(proto, buttonPositionAndVelocity, buttonState)
    local colour = buttonState and {r = 150, g = 255, b = 150} or {r = 255, g = 150, b = 150}

    return calculateBidirectionalGlitz(400, constructDynamicArea(proto, buttonPositionAndVelocity), colour, colour)
end

local function calculateBeamGlitz(proto, beamPositionAndVelocity, buttonState)
    if buttonState then
        return false
    end
    local colour = {r = 255, g = 0, b = 0}

    return calculateBidirectionalGlitz(400, constructDynamicArea(proto, beamPositionAndVelocity), colour, colour)
end

local function momentarySwitch(p)
    local justPressed = nil
    local justReleased = nil
    local PnV = nil
    local state = nil
    
    local triggerID = p.triggerID
    local stateTriggerID = p.stateTriggerID or p.triggerID

    local proto = {
        timeDirection = p.timeDirection,
        attachment = cloneAttachment(p.attachment),
        width = p.width,
        height = p.height,
    }
    
    return {
        calcPnV = function(self, collisions)
            PnV = calculateButtonPositionAndVelocity(proto, collisions)
        end,
        updateState = function(self, departures, triggerArrivals)
            local oldState = triggerArrivals[stateTriggerID][1]
            state = (checkPressed(constructDynamicArea(proto, PnV), departures) and oldState + 1) or 0
            if state > 2 then
                state = 2
            end
            justPressed = oldState == 0 and state > 0
            justReleased = oldState > 0 and state == 0
        end,
        calculateGlitz = function(self, forwardsGlitz, reverseGlitz, persistentGlitz)
            local forGlitz, revGlitz = calculateButtonGlitz(proto, PnV, state > 0)
            table.insert(forwardsGlitz, forGlitz)
            table.insert(reverseGlitz, revGlitz)
            if justPressed then
                table.insert(
                    persistentGlitz,
                    {type = "audio", key = "global.switch_push_down", duration = 10, timeDirection = proto.timeDirection})
            end
            if justReleased then
                table.insert(
                    persistentGlitz,
                    {type = "audio", key = "global.switch_push_up", duration = 10, timeDirection = proto.timeDirection})
            end
        end,
        fillTrigger = function(self, outputTriggers)
            outputTriggers[triggerID] = {state}
            outputTriggers[stateTriggerID] = {state}
            if p.extraTriggerIDs then
                local extraTriggerIDs = p.extraTriggerIDs
                for i = 1, #extraTriggerIDs do
                    outputTriggers[extraTriggerIDs[i]] = {state}
                end
            end
        end,
    }
end

local function stickySwitch(p)
    local PnV = nil
    local state = nil

    local justPressed = nil

    local triggerID = p.triggerID
    local stateTriggerID = p.stateTriggerID or p.triggerID
    local proto = {
        timeDirection = p.timeDirection,
        attachment = cloneAttachment(p.attachment),
        width = p.width,
        height = p.height,
    }
    
    return {
        calcPnV = function(self, collisions)
            PnV = calculateButtonPositionAndVelocity(proto, collisions)
        end,
        updateState = function(self, departures, triggerArrivals)
            local oldState = triggerArrivals[stateTriggerID][1]
            state = (oldState > 0 and oldState + 1) or (checkPressed(constructDynamicArea(proto, PnV), departures) and 1) or 0
            if state > 2 then
                state = 2
            end
            justPressed = oldState == 0 and state > 0
        end,
        calculateGlitz = function(self, forwardsGlitz, reverseGlitz, persistentGlitz)
            local forGlitz, revGlitz = calculateButtonGlitz(proto, PnV, state > 0)
            table.insert(forwardsGlitz, forGlitz)
            table.insert(reverseGlitz, revGlitz)
            if justPressed then
                table.insert(
                    persistentGlitz,
                    {type = "audio", key = "global.switch_push_down", duration = 10, timeDirection = proto.timeDirection})
            end
        end,
        fillTrigger = function(self, outputTriggers)
            outputTriggers[triggerID] = {state}
            outputTriggers[stateTriggerID] = {state}
            if p.extraTriggerIDs then
                local extraTriggerIDs = p.extraTriggerIDs
                for i = 1, #extraTriggerIDs do
                    outputTriggers[extraTriggerIDs[i]] = {state}
                end
            end
        end,
    }
end

local function stickyLaserSwitch(p)
    local beamPnV = nil
    local emitterPnV = nil
    local state = nil

    local justPressed = nil

    local triggerID = p.triggerID
    local stateTriggerID = p.stateTriggerID or p.triggerID

    local timeDirection = p.timeDirection
    local attachment = p.attachment
    local beamLength = p.beamLength
    local beamDirection = p.beamDirection
    local beamWidth = 120

    local emitterLength = 400
    local emitterWidth = 1200

    local protoBeam
    local protoEmitter

    if beamDirection == 0 then -- Right
        protoBeam = {
            timeDirection = timeDirection,
            attachment = cloneAttachment(attachment),
            width = beamLength,
            height = beamWidth,
        }
        protoBeam.attachment.yOffset = protoBeam.attachment.yOffset - beamWidth/2
        
        protoEmitter = {
            timeDirection = timeDirection,
            attachment = cloneAttachment(attachment),
            height = emitterWidth,
            width = emitterLength,
        }
        protoEmitter.attachment.yOffset = protoEmitter.attachment.yOffset - emitterWidth/2
    elseif beamDirection == 1 then -- Down (following right hand rule)
        protoBeam = {
            timeDirection = timeDirection,
            attachment = cloneAttachment(attachment),
            width = beamWidth,
            height = beamLength,
        }
        protoBeam.attachment.xOffset = protoBeam.attachment.xOffset - beamWidth/2
        
        protoEmitter = {
            timeDirection = timeDirection,
            attachment = cloneAttachment(attachment),
            height = emitterLength,
            width = emitterWidth,
        }
        protoEmitter.attachment.xOffset = protoEmitter.attachment.xOffset - emitterWidth/2
    elseif beamDirection == 2 then -- Left
        protoBeam = {
            timeDirection = timeDirection,
            attachment = cloneAttachment(attachment),
            width = beamLength,
            height = beamWidth,
        }
        protoBeam.attachment.xOffset = protoBeam.attachment.xOffset - beamLength
        protoBeam.attachment.yOffset = protoBeam.attachment.yOffset - beamWidth/2
        
        protoEmitter = {
            timeDirection = timeDirection,
            attachment = cloneAttachment(attachment),
            height = emitterWidth,
            width = emitterLength,
        }
        protoEmitter.attachment.xOffset = protoEmitter.attachment.xOffset - emitterLength
        protoEmitter.attachment.yOffset = protoEmitter.attachment.yOffset - emitterWidth/2
    else -- Up
        protoBeam = {
            timeDirection = timeDirection,
            attachment = cloneAttachment(attachment),
            width = beamWidth,
            height = beamLength,
        }
        protoBeam.attachment.xOffset = protoBeam.attachment.xOffset - beamWidth/2
        protoBeam.attachment.yOffset = protoBeam.attachment.yOffset - beamLength
        
        protoEmitter = {
            timeDirection = timeDirection,
            attachment = cloneAttachment(attachment),
            height = emitterLength,
            width = emitterWidth,
        }
        protoEmitter.attachment.xOffset = protoEmitter.attachment.xOffset - emitterWidth/2
        protoEmitter.attachment.yOffset = protoEmitter.attachment.yOffset - emitterLength
    end

    return {
        calcPnV = function(self, collisions)
            beamPnV = calculateButtonPositionAndVelocity(protoBeam, collisions)
            emitterPnV = calculateButtonPositionAndVelocity(protoEmitter, collisions)
        end,
        updateState = function(self, departures, triggerArrivals)
            local oldState = triggerArrivals[stateTriggerID][1]
            state = (oldState > 0 and oldState + 1) or (checkPressed(constructDynamicArea(protoBeam, beamPnV), departures) and 1) or 0
            if state > 2 then
                state = 2
            end
            justPressed = oldState == 0 and state > 0
        end,
        calculateGlitz = function(self, forwardsGlitz, reverseGlitz, persistentGlitz)
            local forGlitzBeam, revGlitzBeam = calculateBeamGlitz(protoBeam, beamPnV, state > 0)
            if forGlitzBeam then
                table.insert(forwardsGlitz, forGlitzBeam)
                table.insert(reverseGlitz, revGlitzBeam)
            end
            
            local forGlitz, revGlitz = calculateButtonGlitz(protoEmitter, emitterPnV, state > 0)
            table.insert(forwardsGlitz, forGlitz)
            table.insert(reverseGlitz, revGlitz)
            if justPressed then
                table.insert(
                    persistentGlitz,
                    {type = "audio", key = "global.switch_push_down", duration = 10, timeDirection = timeDirection})
            end
        end,
        fillTrigger = function(self, outputTriggers)
            outputTriggers[triggerID] = {state}
            outputTriggers[stateTriggerID] = {state}
            if p.extraTriggerIDs then
                local extraTriggerIDs = p.extraTriggerIDs
                for i = 1, #extraTriggerIDs do
                    outputTriggers[extraTriggerIDs[i]] = {state}
                end
            end
        end,
    }
end

local function multiStickySwitch(p)
    local PnV = {}
    local state = nil
    local individualState = {}
    local buttons = p.buttons
    local bCount = #p.buttons
    local proto = {}
    
    local triggerID = p.triggerID
    local stateTriggerID = p.stateTriggerID
    
    local justPressed = {}
    local justReleased = {}

    for i = 1, bCount do
        proto[i] = {
            timeDirection = p.timeDirection,
            attachment = cloneAttachment(buttons[i].attachment),
            width = buttons[i].width,
            height = buttons[i].height,
        }
    end
    
    return {
        calcPnV = function(self, collisions)
            for i = 1, bCount do
                PnV[i] = calculateButtonPositionAndVelocity(proto[i], collisions)
            end
        end,
        updateState = function(self, departures, triggerArrivals)
            state = true
            for i = 1, bCount do
                if triggerArrivals[stateTriggerID][i] == 0 then
                    state = false
                    break
                end
            end

            if not state then
                state = true
                for i = 1, bCount do
                    individualState[i] = checkPressed(constructDynamicArea(proto[i], PnV[i]), departures)
                    if not individualState[i] then 
                        state = false
                    end
                    justPressed[i] = triggerArrivals[stateTriggerID][i] == 0 and individualState[i]
                    justReleased[i] = triggerArrivals[stateTriggerID][i] > 0 and not individualState[i]
                end
            else
                for i = 1, bCount do
                    justPressed[i] = false
                    justReleased[i] = false
                end
            end
            
            state = (state and triggerArrivals[triggerID][1] + 1) or 0
            if state > 2 then
                state = 2
            end
        end,
        calculateGlitz = function(self, forwardsGlitz, reverseGlitz, persistentGlitz)
            for i = 1, bCount do
                local forGlitz, revGlitz = calculateButtonGlitz(proto[i], PnV[i], (state > 0) or individualState[i])
                table.insert(forwardsGlitz, forGlitz)
                table.insert(reverseGlitz, revGlitz)
                if justPressed[i] then
                    table.insert(
                        persistentGlitz,
                        {type = "audio", key = "global.switch_push_down", duration = 10, timeDirection = proto[i].timeDirection})
                end
                if justReleased[i] then
                    table.insert(
                        persistentGlitz,
                        {type = "audio", key = "global.switch_push_up", duration = 10, timeDirection = proto[i].timeDirection})
                end
            end
        end,
        fillTrigger = function(self, outputTriggers)
            outputTriggers[triggerID] = {state}
            outputTriggers[stateTriggerID] = map(function(val) return val and 1 or 0 end, individualState)
            if p.extraTriggerIDs then
                local extraTriggerIDs = p.extraTriggerIDs
                for i = 1, #extraTriggerIDs do
                    outputTriggers[extraTriggerIDs[i]] = {state}
                end
            end
        end,
    }
end

local function toggleSwitch(p)
    local function cloneButtonSegment(q)
        return {
            attachment = cloneAttachment(q.attachment),
            width = q.width,
            height = q.height,
        }
    end
    local justPressed = nil
    local firstPnV = nil
    local secondPnV = nil
    local switchState = nil
    local triggerID = p.triggerID
    local stateTriggerID = p.stateTriggerID or p.triggerID
    local timeDirection = p.timeDirection

    local function constructCompleteProto(timeDirection, segment)
        return {
            timeDirection = timeDirection,
            attachment = segment.attachment,
            width = segment.width, height = segment.height,
            x = segment.x, y = segment.y}
    end
    return
    {
        calcPnV = function(self, collisions)
            firstPnV = calculateButtonPositionAndVelocity(constructCompleteProto(timeDirection, self.first), collisions)
            secondPnV = calculateButtonPositionAndVelocity(constructCompleteProto(timeDirection, self.second), collisions)
        end,
        updateState = function(self, departures, triggerArrivals)
            local firstPressed = checkPressed(constructDynamicArea(constructCompleteProto(timeDirection, self.first), firstPnV), departures)
            local secondPressed = checkPressed(constructDynamicArea(constructCompleteProto(timeDirection, self.second), secondPnV), departures)
            
            if firstPressed and secondPressed then
                switchState = triggerArrivals[stateTriggerID][1]
            elseif triggerArrivals[stateTriggerID][1] == 0 then
                if firstPressed then switchState = 1 else switchState = 0 end
            else
                if secondPressed then switchState = 0 else switchState = 1 end
            end
            justPressed = switchState ~= triggerArrivals[stateTriggerID][1]
            
        end,
        calculateGlitz = function(self, forwardsGlitz, reverseGlitz, persistentGlitz)
            do
                local forGlitz, revGlitz =
                    calculateButtonGlitz(constructCompleteProto(timeDirection, self.first), firstPnV, switchState > 0)
                table.insert(forwardsGlitz, forGlitz)
                table.insert(reverseGlitz, revGlitz)
            end
            do
                local forGlitz, revGlitz =
                    calculateButtonGlitz(constructCompleteProto(timeDirection, self.second), secondPnV, switchState == 0)
                table.insert(forwardsGlitz, forGlitz)
                table.insert(reverseGlitz, revGlitz)
            end
            if justPressed then
                table.insert(
                    persistentGlitz,
                    {type = "audio", key = "global.switch_toggle", duration = 6, timeDirection = timeDirection})
            end
        end,
        fillTrigger = function(self, outputTriggers)
            outputTriggers[triggerID] = {switchState}
            outputTriggers[stateTriggerID] = {switchState}
            if p.extraTriggerIDs then
                local extraTriggerIDs = p.extraTriggerIDs
                for i = 1, #extraTriggerIDs do
                    outputTriggers[extraTriggerIDs[i]] = {switchState}
                end
            end
        end,
        first = cloneButtonSegment(p.first),
        second = cloneButtonSegment(p.second)
    }  
end

local pickupGlitzNameMap = {
    timeJump = "global.time_jump",
    timeReverse = "global.time_reverse",
    timeGun = "global.time_gun",
    timePause = "global.time_pause"
}

local function pickup(p)

    local PnV = nil
    local active = true
    local triggerID = p.triggerID
    local justTaken = false
    local proto = {
        timeDirection = p.timeDirection,
        attachment = cloneAttachment(p.attachment),
        width = p.width,
        height = p.height,
        pickupType = p.pickupType,
        pickupNumber = p.pickupNumber,
    }

    return {
        addMutator = function(self, triggerArrivals, collisions, mutators, activeMutators) 
            if triggerArrivals[triggerID][1] == 1 then
                PnV = calculateButtonPositionAndVelocity(proto, collisions)
                mutators[#mutators+1] = {
                    x = PnV.x, y = PnV.y,
                    width = proto.width, height = proto.height,
                    xspeed = PnV.xspeed, yspeed = PnV.yspeed,
                    collisionOverlap = 0,
                    timeDirection = proto.timeDirection
                }
                activeMutators[#activeMutators+1] = self
            else
                active = false
            end
        end,
        effect = function(self, dynamicObject)
            if not active then 
                return dynamicObject 
            end
            if dynamicObject.type ~= 'guy' then 
                return dynamicObject 
            end
            dynamicObject.pickups[proto.pickupType] =
                dynamicObject.pickups[proto.pickupType] + (proto.pickupNumber or 1)
            active = false
            justTaken = true
            return dynamicObject
        end,
        calculateGlitz = function(self, forwardsGlitz, reverseGlitz, persistentGlitz)
            if active then
                local forwardsImageGlitz = {
                    type = "image",
                    layer = 430,
                    key = pickupGlitzNameMap[proto.pickupType],
                    x = PnV.x,
                    y = PnV.y,
                    width = proto.width,
                    height = proto.height
                }

                local reverseImageGlitz = {
                    type = "image",
                    layer = 430,
                    key = pickupGlitzNameMap[proto.pickupType],
                    x = PnV.x,
                    y = PnV.y,
                    width = proto.width,
                    height = proto.height
                }
                table.insert(forwardsGlitz, forwardsImageGlitz)
                table.insert(reverseGlitz, reverseImageGlitz)
            end
            if justTaken then
                table.insert(
                    persistentGlitz,
                    {type = "audio",
                     key = "global.pickup_pickup",
                     duration = 9,
                     timeDirection = proto.timeDirection})
            end
        end,
        fillTrigger = function(self, outputTriggers)
            outputTriggers[triggerID] = {active and 1 or 0}
        end
    }
end


local powerupGlitzNameMap = {
    jumpBoots = "global.powerup_jump",
}

local powerupEffectNameMap = {
    jumpBoots = function (object)
        object.jumpSpeed = -500
    end
}

local function powerup(p)

    local PnV = nil
    local active = true
    local triggerID = p.triggerID
    local justTaken = false
    local proto = {
        timeDirection = p.timeDirection,
        attachment = cloneAttachment(p.attachment),
        width = p.width,
        height = p.height,
        powerupType = p.powerupType,
    }

    return {
        addMutator = function(self, triggerArrivals, collisions, mutators, activeMutators) 
            if triggerArrivals[triggerID][1] == 1 then
                PnV = calculateButtonPositionAndVelocity(proto, collisions)
                mutators[#mutators+1] = {
                    x = PnV.x, y = PnV.y,
                    width = proto.width, height = proto.height,
                    xspeed = PnV.xspeed, yspeed = PnV.yspeed,
                    collisionOverlap = 0,
                    timeDirection = proto.timeDirection
                }
                activeMutators[#activeMutators+1] = self
            else
                active = false
            end
        end,
        effect = function(self, dynamicObject)
            if not active then 
                return dynamicObject 
            end
            if dynamicObject.type ~= 'guy' then 
                return dynamicObject 
            end
            powerupEffectNameMap[proto.powerupType](dynamicObject)
            active = false
            justTaken = true
            return dynamicObject
        end,
        calculateGlitz = function(self, forwardsGlitz, reverseGlitz, persistentGlitz)
            if active then
                local forwardsImageGlitz = {
                    type = "image",
                    layer = 430,
                    key = powerupGlitzNameMap[proto.powerupType],
                    x = PnV.x,
                    y = PnV.y,
                    width = proto.width,
                    height = proto.height
                }

                local reverseImageGlitz = {
                    type = "image",
                    layer = 430,
                    key = powerupGlitzNameMap[proto.powerupType],
                    x = PnV.x,
                    y = PnV.y,
                    width = proto.width,
                    height = proto.height
                }
                table.insert(forwardsGlitz, forwardsImageGlitz)
                table.insert(reverseGlitz, reverseImageGlitz)
            end
            if justTaken then
                table.insert(
                    persistentGlitz,
                    {type = "audio",
                     key = "global.pickup_pickup",
                     duration = 9,
                     timeDirection = proto.timeDirection})
            end
        end,
        fillTrigger = function(self, outputTriggers)
            outputTriggers[triggerID] = {active and 1 or 0}
        end
    }
end

local function spikes(p)
    local PnV = nil
    local deathGlitz = {}
    local proto = {
        timeDirection = p.timeDirection,
        attachment = cloneAttachment(p.attachment),
        width = p.width,
        height = p.height,
    }
    
    return {
        addMutator = function(self, triggerArrivals, collisions, mutators, activeMutators) 
            PnV = calculateButtonPositionAndVelocity(proto, collisions)
            mutators[#mutators+1] = {
                x = PnV.x, y = PnV.y,
                width = proto.width, height = proto.height,
                xspeed = PnV.xspeed, yspeed = PnV.yspeed,
                collisionOverlap = 1, -- So that spikes just below the surface are not deadly
                timeDirection = proto.timeDirection
            }
            activeMutators[#activeMutators+1] = self
        end,
        calculateGlitz = function(self, forwardsGlitz, reverseGlitz)
            local colour = {r = 255, g = 0, b = 0}
            local forGlitz, revGlitz = calculateBidirectionalGlitz(430, constructDynamicArea(proto, PnV), colour, colour)
            table.insert(forwardsGlitz, forGlitz)
            table.insert(reverseGlitz, revGlitz)
            for i = 1, #deathGlitz do
                local forGlitz, revGlitz = calculateBidirectionalGlitz(430, deathGlitz[i], {r = 255, g = 0, b = 0},  {r = 0, g = 255, b = 255})
                table.insert(forwardsGlitz, forGlitz)
                table.insert(reverseGlitz, revGlitz)
            end
        end,
        effect = function(self, dynamicObject)
            if dynamicObject.type == 'guy' then 
                deathGlitz[#deathGlitz+1] = {
                    x = dynamicObject.x,
                    y = dynamicObject.y,
                    xspeed = dynamicObject.xspeed,
                    yspeed = dynamicObject.yspeed,
                    width = dynamicObject.width,
                    height = dynamicObject.height,
                    timeDirection = dynamicObject.timeDirection
                }
                return nil
            else
                return dynamicObject 
            end
        end,
        fillTrigger = function(self, outputTriggers)
        end
    }
end

local function trampoline(p)

    local PnV = nil
    local active = true
    local proto = {
        timeDirection = p.timeDirection,
        attachment = cloneAttachment(p.attachment),
        width = p.width,
        height = p.height,
        bounce = p.bounce,
    }

    return {
        addMutator = function(self, triggerArrivals, collisions, mutators, activeMutators) 
            PnV = calculateButtonPositionAndVelocity(proto, collisions)
            mutators[#mutators+1] = {
                x = PnV.x, y = PnV.y,
                width = proto.width, height = proto.height,
                xspeed = PnV.xspeed, yspeed = PnV.yspeed,
                collisionOverlap = 0,
                timeDirection = proto.timeDirection
            }
            activeMutators[#activeMutators+1] = self
        end,
        effect = function(self, dynamicObject)
            if dynamicObject.yspeed > 100 then
                dynamicObject.yspeed = -1*proto.bounce*dynamicObject.yspeed
            end
            return dynamicObject
        end,
        calculateGlitz = function(self, forwardsGlitz, reverseGlitz, persistentGlitz)
            local forwardsImageGlitz = {
                type = "image",
                layer = 430,
                key = "global.trampoline",
                x = PnV.x,
                y = PnV.y,
                width = proto.width,
                height = proto.height
            }

            local reverseImageGlitz = {
                type = "image",
                layer = 430,
                key = "global.trampoline",
                x = PnV.x,
                y = PnV.y,
                width = proto.width,
                height = proto.height
            }
            table.insert(forwardsGlitz, forwardsImageGlitz)
            table.insert(reverseGlitz, reverseImageGlitz)
        end,
        fillTrigger = function(self, outputTriggers)
        end
    }
end

local function boxOMatic(p)
    local PnV = nil
    local boxCreationFunction = p.boxCreationFunction
    local proto = {
        timeDirection = p.timeDirection,
        attachment = cloneAttachment(p.attachment),
        width = p.width,
        height = p.height,
    }
    
    return {
        calcPnV = function(self, collisions)
            PnV = calculateButtonPositionAndVelocity(proto, collisions)
        end,
        calculateGlitz = function(self, forwardsGlitz, reverseGlitz)
            local colour = {r = 150, g = 150, b = 255}
            local forGlitz, revGlitz = calculateBidirectionalGlitz(800, constructDynamicArea(proto, PnV), colour, colour)
            local textGlitz = {
                    type = "text",
                    x = PnV.x+150,
                    y = PnV.y+100,
                    text = "Box\n  O\nMatic",
                    size = 1200,
                    layer = 850,
                    colour = {r = 0, g = 0, b = 0},
                }
            table.insert(forwardsGlitz, forGlitz)
            table.insert(reverseGlitz, revGlitz)
            table.insert(forwardsGlitz, textGlitz)
            table.insert(reverseGlitz, textGlitz)
        end,
        getAdditionalBoxes = function(self, triggers, frameNumber, additionalBoxes)
            local newBox = boxCreationFunction(triggers, frameNumber)
            if newBox then
                newBox.x = newBox.x + PnV.x
                newBox.y = newBox.y + PnV.y
                newBox.xspeed = newBox.xspeed + PnV.xspeed
                newBox.yspeed = newBox.yspeed + PnV.yspeed
                
                additionalBoxes[#additionalBoxes+1] = newBox
            end
        end,
    }
end

local function wireGlitz(p)

    local triggerID = p.triggerID
    local triggerFunction = p.triggerFunction
    local useTriggerArrival = p.useTriggerArrival
    local proto = {
        x1 = p.x1, 
        y1 = p.y1,
        x2 = p.x2,
        y2 = p.y2
    }

    return {
        calculateGlitz = function(self, forwardsGlitz, reverseGlitz, physicsAffectingStuff, triggerArrivals, outputTriggers)

            local collisions = physicsAffectingStuff.collisions
        
            local x1 = proto.x1.pos + ((proto.x1.platform ~= nil and collisions[proto.x1.platform].x) or 0)
            local y1 = proto.y1.pos + ((proto.y1.platform ~= nil and collisions[proto.y1.platform].y) or 0)
            local x2 = proto.x2.pos + ((proto.x2.platform ~= nil and collisions[proto.x2.platform].x) or 0)
            local y2 = proto.y2.pos + ((proto.y2.platform ~= nil and collisions[proto.y2.platform].y) or 0)
        
            local obj = {
                x = (x1 < x2 and x1) or x2,
                y = (y1 < y2 and y1) or y2,
                width = (x1 < x2 and x2 - x1) or x1 - x2,
                height = (y1 < y2 and y2 - y1) or y1 - y2,
            }
            
            local forGlitz, revGlitz
            local active = false
            if triggerID then
                active = (useTriggerArrival and triggerArrivals[triggerID][1] > 0) or (not useTriggerArrival and outputTriggers[triggerID][1] > 0)
            elseif triggerFunction then
                active = triggerFunction(triggerArrivals, outputTriggers)
            end
            
            if active then
                forGlitz, revGlitz = calculateBidirectionalGlitz(1500, obj, {r = 0, g = 180, b = 0}, {r = 0, g = 180, b = 0})
            else
                forGlitz, revGlitz = calculateBidirectionalGlitz(1500, obj, {r = 180, g = 0, b = 0}, {r = 180, g = 0, b = 0})
            end
            
            table.insert(forwardsGlitz, forGlitz)
            table.insert(reverseGlitz, revGlitz)
        end,
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
    local colour = p.colour
    local layer = p.layer
    local proto = {
        x = p.x,
        y = p.y,
        width = p.width,
        height = p.height,
    }
    return {
        calculateGlitz = function(self, forwardsGlitz, reverseGlitz)
            local forGlitz, revGlitz = calculateBidirectionalGlitz(layer, proto, colour, colour)
            table.insert(forwardsGlitz, forGlitz)
            table.insert(reverseGlitz, revGlitz)
        end,
    }
end

local function basicTextGlitz(p)
    local proto = {
        type = "text",
        x = p.x,
        y = p.y,
        text = p.text,
        size = p.size,
        layer = p.layer,
        colour = p.colour,
    }
    return {
        calculateGlitz = function(self, forwardsGlitz, reverseGlitz)
            table.insert(forwardsGlitz, proto)
            table.insert(reverseGlitz, proto)
        end,
    }
end

local function calculateMutators(protoMutators, collisions, triggerArrivals)
    local mutators = {}
    --Ordered list of protoMutators that
    --actually added mutators to `mutators`.
    --(1-1 correspondence between index of
    -- mutators and the responsible protoMutators)
    local activeMutators = {}
    for protoMutator in list_iter(protoMutators) do
        protoMutator:addMutator(triggerArrivals, collisions, mutators, activeMutators)
    end
    return mutators, activeMutators
end

function calculatePhysicsAffectingStuff(tempStore)
    return function (frameNumber, triggerArrivals)
        local retv = {}

        tempStore.outputTriggers = {}
        tempStore.forwardsGlitz = {}
        tempStore.reverseGlitz = {}
        tempStore.persistentGlitz = {}
        tempStore.frameNumber = frameNumber
        tempStore.triggerArrivals = triggerArrivals

        retv.additionalBoxes = {}
        tempStore.additionalEndBoxes = {}

        if tempStore.triggerManipulationFunction then
            tempStore.triggerManipulationFunction(triggerArrivals, tempStore.outputTriggers, tempStore.frameNumber)
        end
        
        retv.collisions = calculateCollisions(tempStore.protoCollisions, triggerArrivals, tempStore.outputTriggers, tempStore.frameNumber)
        
        retv.mutators, tempStore.activeMutators = calculateMutators(tempStore.protoMutators, retv.collisions, triggerArrivals)
        
        local portals, arrivalLocations = calculatePortals(
            tempStore.forwardsGlitz, 
            tempStore.reverseGlitz, 
            tempStore.protoPortals, 
            --tempStore.protoLasers, 
            retv.collisions, 
            triggerArrivals, 
            tempStore.frameNumber
        )
        
        retv.portals = portals
        retv.arrivalLocations = arrivalLocations
        
        calculateButtonPositionsAndVelocities(tempStore.protoButtons, retv.collisions)

        fillCollisionTriggers(tempStore.outputTriggers, tempStore.protoCollisions, retv.collisions)

        for collision in list_iter(retv.collisions) do
            local forwardsGlitz, reverseGlitz = calculateCollisionGlitz(collision)
            table.insert(tempStore.forwardsGlitz, forwardsGlitz)
            table.insert(tempStore.reverseGlitz, reverseGlitz)
        end
        
        if tempStore.protoBoxCreators then
            for protoBoxCreator in list_iter(tempStore.protoBoxCreators) do
                protoBoxCreator:calcPnV(retv.collisions)
                protoBoxCreator:calculateGlitz(tempStore.forwardsGlitz, tempStore.reverseGlitz)
                protoBoxCreator:getAdditionalBoxes(tempStore.triggerArrivals, tempStore.frameNumber, retv.additionalBoxes)
            end
        end
        
        tempStore.physicsAffectingStuff = retv

        return retv
    end
end

local function mutateObject(tempStore)
    return function(responsibleMutatorIndices, dynamicObject)
        for i in list_iter(responsibleMutatorIndices) do
            if not dynamicObject then
                break
            end
            dynamicObject = tempStore.activeMutators[i]:effect(dynamicObject)
        end
        return dynamicObject
    end
end


local function getDepartureInformation(tempStore)
    return function(departures)
        calculateButtonStates(tempStore.protoButtons, departures, tempStore.triggerArrivals)

        for protoButton in list_iter(tempStore.protoButtons) do
            protoButton:calculateGlitz(tempStore.forwardsGlitz, tempStore.reverseGlitz, tempStore.persistentGlitz)
            protoButton:fillTrigger(tempStore.outputTriggers)
        end

        for protoMutator in list_iter(tempStore.protoMutators) do
            protoMutator:calculateGlitz(tempStore.forwardsGlitz, tempStore.reverseGlitz, tempStore.persistentGlitz)
            protoMutator:fillTrigger(tempStore.outputTriggers)
        end
        
        if tempStore.protoGlitz then
            for protoGlitz in list_iter(tempStore.protoGlitz) do
                protoGlitz:calculateGlitz(tempStore.forwardsGlitz, tempStore.reverseGlitz, tempStore.physicsAffectingStuff, tempStore.triggerArrivals, tempStore.outputTriggers)
            end
        end

        return
            tempStore.outputTriggers,
            tempStore.forwardsGlitz,
            tempStore.reverseGlitz,
            tempStore.persistentGlitz,
            tempStore.additionalEndBoxes
    end
end

local function timeDirectionToInt(dir)
    if dir == 'forwards' then
        return 1
    elseif dir == 'reverse' then
        return -1
    else
        return 0
    end
end

return {
    momentarySwitch = momentarySwitch,
    toggleSwitch = toggleSwitch,
    stickySwitch = stickySwitch,
    stickyLaserSwitch = stickyLaserSwitch,
    multiStickySwitch = multiStickySwitch,
    pickup = pickup,
    powerup = powerup,
    spikes = spikes,
    trampoline = trampoline,
    boxOMatic = boxOMatic,
    wireGlitz = wireGlitz,
    easyWireGlitz = easyWireGlitz,
    basicRectangleGlitz = basicRectangleGlitz,
    basicTextGlitz = basicTextGlitz,
    mutateObject = mutateObject,
    calculatePhysicsAffectingStuff = calculatePhysicsAffectingStuff,
    getDepartureInformation = getDepartureInformation,
    calculateBidirectionalGlitz = calculateBidirectionalGlitz,
    timeDirectionToInt = timeDirectionToInt,
}
