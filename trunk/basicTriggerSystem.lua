
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
    local collisions = {}
    for i, protoCollision in ipairs(protoCollisions) do
        collisions[i] = calculateCollision(protoCollision, triggerArrivals)
    end
    return collisions
end

local function list_iter (t)
    local i = 0
    local n = #t
    return function ()
        i = i + 1
        if i <= n then return t[i] end
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
    arrivalLocations = {}
    for i, portal in ipairs(portals) do
        arrivalLocations[i] = calculateArrivalLocation(portal)
    end
    return arrivalLocations
end

local function calculateButtonPositionsAndVelocities(protoButtons, collisions)
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
    buttonPositionsAndVelocities = {}
    for i, protoButton in ipairs(protoButtons) do
        buttonPositionsAndVelocities[i] = calculateButtonPositionAndVelocity(protoButton, collisions)
    end
    return buttonPositionsAndVelocities
end

local function calculateButtonStates(protoButtons, buttonPositionsAndVelocities, departures)
    local function temporalIntersectingExclusive(protoButton, buttonPosition, object)
        local xa, ya
        if protoButton.timeDirection == 'forwards' then
            xa = buttonPosition.x
            ya = buttonPosition.y
        else
            xa = buttonPosition.x - buttonPosition.xspeed
            ya = buttonPosition.y - buttonPosition.yspeed
        end
        local wa, ha = protoButton.width, protoButton.height
        local xb, yb
        if object.timeDirection == 'forwards' then
            xb = object.x
            yb = object.y
        else 
            xb = object.x - object.xspeed
            yb = object.y - object.yspeed
        end
        local wb, hb = object.width, object.height
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
    local buttonStates = {}
    for i = 1, #protoButtons do
        local intersecting = false
        for frame, objectList in pairs(departures) do
            if intersecting then break end
            for box in list_iter(objectList.boxes) do
                if intersecting 
                    or temporalIntersectingExclusive(
                        protoButtons[i], buttonPositionsAndVelocities[i], box)
                then
                    intersecting = true
                    break
                end
            end
            for guy in list_iter(objectList.guys) do
                if intersecting 
                    or temporalIntersectingExclusive(
                        protoButtons[i], buttonPositionsAndVelocities[i], guy)
                then 
                    intersecting = true
                    break
                end
            end
        end
        buttonStates[i] = intersecting
    end
    return buttonStates
end

local function fillCollisionTriggers(triggers, protoCollisions, collisions)
    for i = 1, #protoCollisions do
        triggers[protoCollisions[i].lastStateTriggerID] =
            {collisions[i].x, collisions[i].y, collisions[i].xspeed, collisions[i].yspeed}
    end
end

local function calculateBidirectionalGlitz(layer, obj, forwardsColour, reverseColour)
    local sameDirectionGlitz = {
        layer = layer,
        x = obj.x, y = obj.y,
        width = obj.width, height = obj.height,
        colour = forwardsColour
    }
    local oppositeDirectionGlitz = {
        layer = layer,
        x = obj.x - obj.xspeed, y = obj.y - obj.yspeed,
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

local function calculateButtonGlitz(protoButton, buttonPositionAndVelocity, buttonState)
    local colour = buttonState and {r = 150, g = 255, b = 150} or {r = 255, g = 150, b = 150}
    
    return calculateBidirectionalGlitz(
        400,
        {x = buttonPositionAndVelocity.x,
         y = buttonPositionAndVelocity.y,
         width = protoButton.width,
         height = protoButton.height,
         xspeed = buttonPositionAndVelocity.xspeed,
         yspeed = buttonPositionAndVelocity.yspeed,
         timeDirection = protoButton.timeDirection},
        colour,
        colour)
end

local function fillButtonTriggers(triggers, protoButtons, buttonStates)
    for i = 1, #protoButtons do
        triggers[protoButtons[i].triggerID] = { buttonStates[i] and 1 or 0 }
    end
end

local function calculatePhysicsAffectingStuff(tempStore)
    return function (frameNumber, triggerArrivals)
        local retv = {}
    
        tempStore.frameNumber = frameNumber
    
        retv.additionalBoxes = {}
        tempStore.additionalEndBoxes = {}
    
        retv.collisions = calculateCollisions(tempStore.protoCollisions, triggerArrivals)
        retv.portals = calculatePortals(tempStore.protoPortals, retv.collisions)
        retv.mutators = {} -- TODO
        retv.arrivalLocations = calculateArrivalLocations(retv.portals)
    
        tempStore.buttonPositionsAndVelocities =
            calculateButtonPositionsAndVelocities(tempStore.protoButtons, retv.collisions)
     
        fillCollisionTriggers(tempStore.outputTriggers, tempStore.protoCollisions, retv.collisions)
    
        for collision in list_iter(retv.collisions) do
            local forwardsGlitz, reverseGlitz = calculateCollisionGlitz(collision)
            table.insert(tempStore.forwardsGlitz, forwardsGlitz)
            table.insert(tempStore.reverseGlitz, reverseGlitz)
        end
    
        for portal in list_iter(retv.portals) do
            local forwardsGlitz, reverseGlitz = calculatePortalGlitz(portal)
            table.insert(tempStore.forwardsGlitz, forwardsGlitz)
            table.insert(tempStore.reverseGlitz, reverseGlitz)
        end
    
        return retv
    end
end
local function getDepartureInformation(tempStore)
    return function(departures)
        local buttonStates =
            calculateButtonStates(
                tempStore.protoButtons,
                tempStore.buttonPositionsAndVelocities,
                departures)
        
        for i = 1, #tempStore.protoButtons do
            local forwardsGlitz, reverseGlitz =
                calculateButtonGlitz(
                    tempStore.protoButtons[i],
                    tempStore.buttonPositionsAndVelocities[i],
                    buttonStates[i])
            table.insert(tempStore.forwardsGlitz, forwardsGlitz)
            table.insert(tempStore.reverseGlitz, reverseGlitz)
        end
        
        table.insert(tempStore.forwardsGlitz, forwardsGlitz)
        table.insert(tempStore.reverseGlitz, reverseGlitz)
        
        fillButtonTriggers(
            tempStore.outputTriggers, tempStore.protoButtons, buttonStates)
        
        return tempStore.outputTriggers,
               tempStore.forwardsGlitz,
               tempStore.reverseGlitz,
               tempStore.additionalEndBoxes
    end
end

return
{
    getDepartureInformation = getDepartureInformation,
    calculatePhysicsAffectingStuff = calculatePhysicsAffectingStuff,
    calculateBidirectionalGlitz = calculateBidirectionalGlitz,
}