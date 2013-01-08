local function map(f, l)
    local r = {}
    for i, v in ipairs(l) do
        r[i] = f(v)
    end
    return r
end

local function calculateCollisions(protoCollisions, triggerArrivals)
    local function calculateCollision(self, triggerArrivals)
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
                    velocity = velocity + (math.abs(velocity)/velocity)*deceleration
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
        
        local destination = 
            triggerArrivals[self.buttonTriggerID][1] == 0 and
                self.destinations.offDestination or self.destinations.onDestination

        local lastStateTrigger = triggerArrivals[self.lastStateTriggerID]
        
        local horisontalPosAndVel = solvePDEquation(destination.xDestination, lastStateTrigger[1], lastStateTrigger[3])
        local verticalPosAndVel = solvePDEquation(destination.yDestination, lastStateTrigger[2], lastStateTrigger[4])
        
        return
        {
            x = horisontalPosAndVel.position,
            y = verticalPosAndVel.position,
            xspeed = horisontalPosAndVel.velocity,
            yspeed = verticalPosAndVel.velocity,
            width = self.width,
            height = self.height,
            timeDirection = self.timeDirection
        }
    end
    return map(function(protoCollision) return calculateCollision(protoCollision, triggerArrivals) end, protoCollisions)
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

local function calculatePortals(protoPortals, collisions)
    local function calculatePortal(protoPortal, collisions)
        local x, y, xspeed, yspeed =
            snapAttachment(protoPortal.timeDirection, protoPortal.attachment, collisions)
        
        return
        {
            index = protoPortal.index,
            x = x,
            y = y,
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
            illegalDestination = protoPortal.illegalDestination,
            fallable = protoPortal.fallable,
            winner = protoPortal.winner
        }
    end

    local portals = {}
    for i, protoPortal in ipairs(protoPortals) do
        portals[i] = calculatePortal(protoPortal, collisions)
    end
    return portals
end

local function calculateArrivalLocations(portals)
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
    return map(calculateArrivalLocation, portals)
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

local function calculateBidirectionalGlitz(layer, obj, forwardsColour, reverseColour)
    local sameDirectionGlitz = {
        layer = layer,
        type = "rectangle",
        x = obj.x, y = obj.y,
        width = obj.width, height = obj.height,
        colour = forwardsColour
    }
    local oppositeDirectionGlitz = {
        layer = layer,
        type = "rectangle",
        x = obj.x, y = obj.y,
        width = obj.width, height = obj.height,
        colour = reverseColour
    }
    if obj.timeDirection == 'forwards' then
        return sameDirectionGlitz, oppositeDirectionGlitz
    else
        return oppositeDirectionGlitz, sameDirectionGlitz
    end
end

local function calculateCollisionGlitz(collision)
    return calculateBidirectionalGlitz(300, collision, {r = 50, g = 0, b = 0}, {r = 0, g = 0, b = 50})
end

local function calculatePortalGlitz(portal)
    return calculateBidirectionalGlitz(350, portal, {r = 120, g = 120, b = 120}, {r = 120, g = 120, b = 120})
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
    local xa, ya
    if a.timeDirection == 'forwards' then
        xa = a.x
        ya = a.y
    else
        xa = a.x - a.xspeed
        ya = a.y - a.yspeed
    end
    local wa, ha = a.width, a.height
    local xb, yb
    if b.timeDirection == 'forwards' then
        xb = b.x
        yb = b.y
    else
        xb = b.x - b.xspeed
        yb = b.y - b.yspeed
    end
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
            if temporalIntersectingExclusive(buttonArea, box) then --Done
                return true
            end
        end
        for guy in list_iter(objectList.guys) do
            if temporalIntersectingExclusive(buttonArea, guy) then --Done
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


local function momentarySwitch(p)
    local PnV = nil
    local state = nil
    
    local triggerID = p.triggerID
    local proto = {
        timeDirection = p.timeDirection,
        attachment = cloneAttachment(p.attachment), --Done
        width = p.width,
        height = p.height,
    }
    
    return {
        calcPnV = function(self, collisions)
            PnV = calculateButtonPositionAndVelocity(proto, collisions) --Done
        end,
        updateState = function(self, departures, triggerArrivals)
            state = checkPressed(constructDynamicArea(proto, PnV), departures) --Done
        end,
        calculateGlitz = function(self, forwardsGlitz, reverseGlitz)
            local forGlitz, revGlitz = calculateButtonGlitz(proto, PnV, state) -- Done
            table.insert(forwardsGlitz, forGlitz)
            table.insert(reverseGlitz, revGlitz)
        end,
        fillTrigger = function(self, outputTriggers)
            outputTriggers[triggerID] = {state and 1 or 0}
        end,
    }
end

local function stickySwitch(p)
    local PnV = nil
    local state = nil
    
    local triggerID = p.triggerID
    local proto = {
        timeDirection = p.timeDirection,
        attachment = cloneAttachment(p.attachment), --Done
        width = p.width,
        height = p.height,
    }
    
    return {
        calcPnV = function(self, collisions)
            PnV = calculateButtonPositionAndVelocity(proto, collisions) --Done
        end,
        updateState = function(self, departures, triggerArrivals)
            state = triggerArrivals[triggerID][1] == 1 or checkPressed(constructDynamicArea(proto, PnV), departures) --Done
        end,
        calculateGlitz = function(self, forwardsGlitz, reverseGlitz)
            local forGlitz, revGlitz = calculateButtonGlitz(proto, PnV, state) -- Done
            table.insert(forwardsGlitz, forGlitz)
            table.insert(reverseGlitz, revGlitz)
        end,
        fillTrigger = function(self, outputTriggers)
            outputTriggers[triggerID] = {state and 1 or 0}
        end,
    }
end

local function toggleSwitch(p)
    local function cloneButtonSegment(q)
        return {
            attachment = cloneAttachment(q.attachment), --Done
            width = q.width,
            height = q.height,
        }
    end
    local firstPnV = nil
    local secondPnV = nil
    local switchState = nil
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
            firstPnV = calculateButtonPositionAndVelocity(constructCompleteProto(self.timeDirection, self.first), collisions)
            secondPnV = calculateButtonPositionAndVelocity(constructCompleteProto(self.timeDirection, self.second), collisions)
        end,
        updateState = function(self, departures, triggerArrivals)
            local firstPressed = checkPressed(constructDynamicArea(constructCompleteProto(self.timeDirection, self.first), firstPnV), departures)
            local secondPressed = checkPressed(constructDynamicArea(constructCompleteProto(self.timeDirection, self.second), secondPnV), departures)
            
            if firstPressed and secondPressed then
                switchState = triggerArrivals[self.triggerID][1]
                return
            end
            
            if triggerArrivals[self.triggerID][1] == 0 then
                if firstPressed then switchState = 1 else switchState = 0 end
            else
                if secondPressed then switchState = 0 else switchState = 1 end
            end
        end,
        calculateGlitz = function(self, forwardsGlitz, reverseGlitz)
            do
                local forGlitz, revGlitz = calculateButtonGlitz(constructCompleteProto(self.timeDirection, self.first), firstPnV, switchState == 1) -- Done
                table.insert(forwardsGlitz, forGlitz)
                table.insert(reverseGlitz, revGlitz)
            end
            do
                local forGlitz, revGlitz = calculateButtonGlitz(constructCompleteProto(self.timeDirection, self.second), secondPnV, switchState == 0) -- Done
                table.insert(forwardsGlitz, forGlitz)
                table.insert(reverseGlitz, revGlitz)
            end
        end,
        fillTrigger = function(self, outputTriggers)
            outputTriggers[self.triggerID] = {switchState}
        end,
        triggerID = p.triggerID,
        timeDirection = p.timeDirection,
        first = cloneButtonSegment(p.first),
        second = cloneButtonSegment(p.second)
    }  
end

local function pickup(p)
    local active = true
    local triggerID = p.triggerID
    local proto = {
        timeDirection = p.timeDirection,
        x = p.x, y = p.y,
        width = p.width,
        height = p.height,
    }

    return {
        addMutator = function(self, triggerArrivals, mutators, activeMutators) 
            if triggerArrivals[triggerID][1] == 1 then
                mutators[#mutators+1] = {
                    x = proto.x, y = proto.y,
                    width = proto.width, height = proto.height,
                    xspeed = 0, yspeed = 0,
                    collisionOverlap = 0,
                    timeDirection = proto.timeDirection
                }
                activeMutators[#activeMutators+1] = self
            else
                active = false
            end
        end,
        effect = function(self, dynamicObject)
            if not active then return dynamicObject end
            if dynamicObject.type ~= 'guy' then return dynamicObject end
            dynamicObject.pickups[p.pickupType] = dynamicObject.pickups[p.pickupType] + 1
            active = false
            return dynamicObject
        end,
        calculateGlitz = function(self, forwardsGlitz, reverseGlitz)
            if active then
                local forGlitz, revGlitz =
                    calculateBidirectionalGlitz(430, proto, {r = 150, g = 10, b = 150}, {r = 150, g = 10, b = 150})
                table.insert(forwardsGlitz, forGlitz)
                table.insert(reverseGlitz, revGlitz)
            end
        end,
        fillTrigger = function(self, outputTriggers)
            outputTriggers[triggerID] = {active and 1 or 0}
        end
    }
end

local function calculateMutators(protoMutators, triggerArrivals)
    local mutators = {}
    --Ordered list of protoMutators that
    --actually added mutators to `mutators`.
    --(1-1 correspondence between index of
    -- mutators and the responsible protoMutators)
    local activeMutators = {}
    for protoMutator in list_iter(protoMutators) do
        protoMutator:addMutator(triggerArrivals, mutators, activeMutators)
    end
    return mutators, activeMutators
end

function calculatePhysicsAffectingStuff(tempStore)
    return function (frameNumber, triggerArrivals)
        local retv = {}

        tempStore.outputTriggers = {}
        tempStore.forwardsGlitz = {}
        tempStore.reverseGlitz = {}
        tempStore.frameNumber = frameNumber
        tempStore.triggerArrivals = triggerArrivals

        retv.additionalBoxes = {}
        tempStore.additionalEndBoxes = {}

        retv.collisions = calculateCollisions(tempStore.protoCollisions, triggerArrivals) -- Done
        retv.portals = calculatePortals(tempStore.protoPortals, retv.collisions) -- Done
        retv.mutators, tempStore.activeMutators = calculateMutators(tempStore.protoMutators, triggerArrivals) --TODO
        retv.arrivalLocations = calculateArrivalLocations(retv.portals) -- Done

        calculateButtonPositionsAndVelocities(tempStore.protoButtons, retv.collisions) -- Done

        fillCollisionTriggers(tempStore.outputTriggers, tempStore.protoCollisions, retv.collisions) -- Done

        for collision in list_iter(retv.collisions) do -- Done
            local forwardsGlitz, reverseGlitz = calculateCollisionGlitz(collision) -- Done
            table.insert(tempStore.forwardsGlitz, forwardsGlitz)
            table.insert(tempStore.reverseGlitz, reverseGlitz)
        end
    
        for portal in list_iter(retv.portals) do
            local forwardsGlitz, reverseGlitz = calculatePortalGlitz(portal) -- Done
            table.insert(tempStore.forwardsGlitz, forwardsGlitz)
            table.insert(tempStore.reverseGlitz, reverseGlitz)
        end

        return retv
    end
end

local function mutateObject(tempStore)
    return function(responsibleManipulatorIndices, dynamicObject)
        for i in list_iter(responsibleManipulatorIndices) do
            if dynamicObject then
                dynamicObject = tempStore.activeMutators[i]:effect(dynamicObject)
            end
        end
        return dynamicObject
    end
end


local function getDepartureInformation(tempStore)
    return function(departures)
        calculateButtonStates(tempStore.protoButtons, departures, tempStore.triggerArrivals) -- Done

        for protoButton in list_iter(tempStore.protoButtons) do
            protoButton:calculateGlitz(tempStore.forwardsGlitz, tempStore.reverseGlitz) --Done
            protoButton:fillTrigger(tempStore.outputTriggers) --Done
        end

        for protoMutator in list_iter(tempStore.protoMutators) do
            protoMutator:calculateGlitz(tempStore.forwardsGlitz, tempStore.reverseGlitz)
            protoMutator:fillTrigger(tempStore.outputTriggers)
        end

        return tempStore.outputTriggers, tempStore.forwardsGlitz, tempStore.reverseGlitz, tempStore.additionalEndBoxes
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
    pickup = pickup,
    mutateObject = mutateObject,
    calculatePhysicsAffectingStuff = calculatePhysicsAffectingStuff,
    getDepartureInformation = getDepartureInformation,
    calculateBidirectionalGlitz = calculateBidirectionalGlitz,
    timeDirectionToInt = timeDirectionToInt,
}

