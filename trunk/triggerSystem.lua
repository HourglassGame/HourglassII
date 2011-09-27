

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
            yb = object.x - object.yspeed
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

local function calculateCollisionGlitz(collision)
    return
    {
        x = collision.x,
        y = collision.y,
        width = collision.width,
        height = collision.height,
        xspeed = collision.xspeed,
        yspeed = collision.yspeed,
        forwardsColour = {r = 50, g = 0, b = 0},
        reverseColour = {r = 0, g = 0, b = 50},
        timeDirection = collision.timeDirection
    }
end

local function calculatePortalGlitz(portal)
    return
    {
        x = portal.x,
        y = portal.y,
        width = portal.width,
        height = portal.height,
        xspeed = portal.xspeed,
        yspeed = portal.yspeed,
        forwardsColour = {r = 120, g = 120, b = 120},
        reverseColour = {r = 120, g = 120, b = 120},
        timeDirection = portal.timeDirection
    }
end

local function calculateButtonGlitz(protoButton, buttonPositionAndVelocity, buttonState)
    local colour = buttonState and {r = 150, g = 255, b = 150} or {r = 255, g = 150, b = 150}
    return
    {
        x = buttonPositionAndVelocity.x,
        y = buttonPositionAndVelocity.y,
        width = protoButton.width,
        height = protoButton.height,
        xspeed = buttonPositionAndVelocity.xspeed,
        yspeed = buttonPositionAndVelocity.yspeed,
        forwardsColour = colour,
        reverseColour = colour,
        timeDirection = protoButton.timeDirection
    }
end

local function calculateMutatorGlitz(protoMutator)
    return
    {
        x = protoMutator.x,
        y = protoMutator.y,
        width = protoMutator.width,
        height = protoMutator.height,
        xspeed = protoMutator.xspeed,
        yspeed = protoMutator.yspeed,
        forwardsColour = {r = 150, g = 150, b = 150},
        reverseColour = {r = 150, g = 150, b = 150},
        timeDirection = protoMutator.timeDirection
    }
end
    
local function fillButtonTriggers(triggers, protoButtons, buttonStates)
    for i = 1, #protoButtons do
        triggers[protoButtons[i].triggerID] = { buttonStates[i] and 1 or 0 }
    end
end

return {
    --triggerArrivals have already had default values inserted by C++
    --for trigger indices that did not arrive by the time this is called
    calculatePhysicsAffectingStuff = function(self, frameNumber, triggerArrivals)
        local retv = {}
        
        self.frameNumber = frameNumber
        
        retv.additionalBoxes = {}
        self.additionalEndBoxes = {}
        
        self.makeBox = (frameNumber == 2000 and triggerArrivals[3][1] == 0)
        
        --if frameNumber%300 == 0 then
        --   retv.additionalBoxes[#retv.additionalBoxes+1] = {x = 6400, y = 6400, xspeed = 0, yspeed = 0, size = 3200, illegalPortal = nil, arrivalBasis = nil, timeDirection = "forwards"}
        --end
        
        --if frameNumber%2000 == 0 then
        --    retv.additionalBoxes[#retv.additionalBoxes+1] = {x = 12800, y = 6400, xspeed = 600, yspeed = -400, size = 3200, illegalPortal = nil, arrivalBasis = nil, timeDirection = "forwards"}
        --end
        
        retv.collisions = calculateCollisions(self.protoCollisions, triggerArrivals)
        retv.portals = calculatePortals(self.protoPortals, retv.collisions)
        retv.mutators = { [1] = self.protoMutators[1].data}
        retv.arrivalLocations = calculateArrivalLocations(retv.portals)
        
        self.portalActive = (triggerArrivals[3][1] == 1)
        
        self.buttonPositionsAndVelocities =
            calculateButtonPositionsAndVelocities(self.protoButtons, retv.collisions)
         
        fillCollisionTriggers(self.outputTriggers, self.protoCollisions, retv.collisions)
        
        for collision in list_iter(retv.collisions) do
            table.insert(self.outputGlitz, calculateCollisionGlitz(collision))
        end
        
        for portal in list_iter(retv.portals) do
            table.insert(self.outputGlitz, calculatePortalGlitz(portal))
        end
        
        return retv
    end,
    --responsible*Index gives the position in the list of the thing that
    --is responsible for the callin happening.
    --By 'the list' I mean the list of objects returned from calculatePhysicsAffectingStuff
    --corresponding to the callin type (portals/pickups/killers)
    --in particular, this means that this does *not* correspond to the 'index' field
    --of a portal (the 'index' field for identifying illegal portals, but not for this)
    shouldArrive = function(self, dynamicObject)
        return true
    end,
    shouldPort = function(self, responsiblePortalIndex, dynamicObject, porterActionedPortal) 
        return self.portalActive
    end,
    mutateObject = function(self, responsibleManipulatorIndices, dynamicObject)
        return self.protoMutators[1].effect(self, dynamicObject)
    end,
    getDepartureInformation = function(self, departures)
        local buttonStates =
            calculateButtonStates(self.protoButtons, self.buttonPositionsAndVelocities, departures)
    
        for i = 1, #self.protoButtons do
            table.insert(
                self.outputGlitz,
                calculateButtonGlitz(
                    self.protoButtons[i],
                    self.buttonPositionsAndVelocities[i],
                    buttonStates[i]))
        end
        
         table.insert(self.outputGlitz,calculateMutatorGlitz(self.protoMutators[1].data))
        
        fillButtonTriggers(self.outputTriggers, self.protoButtons, buttonStates)
        
        if self.makeBox then
            self.additionalEndBoxes[#self.additionalEndBoxes+1] = {
                box = {x = 12800, y = 6400, xspeed = -600, yspeed = -400, size = 3200, illegalPortal = nil, arrivalBasis = nil, timeDirection = 'forwards'}, 
                targetFrame = 500
            }
        end
        
        return self.outputTriggers, self.outputGlitz, self.additionalBoxes
    end,
    --as an extension, built-in buttons/collision/etc lists (which get run and solved separately)?
    --[[======private data=======]]--
    --mutable store data:
    buttonPositionsAndVelocities = {},
    outputTriggers = {},
    outputGlitz = {},
    
    --constant proto-object data:
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
            winner = false
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
    protoMutators = {
        {
            data = {
                x = 20000,
                y = 32000,
                xspeed = 0,
                yspeed = 0,
                width = 800,
                height = 6400,
                collisionOverlap = 0,
                timeDirection = "forwards",
            },
            effect = function (self, object)
                if object.type == "guy" and object.boxCarrying then
                    --object.boxCarrying = false
                    self.additionalEndBoxes[#self.additionalEndBoxes+1] = {
                        box = {
                            x = 12800, y = 6400, xspeed = 0, yspeed = 0, 
                            size = object.boxCarrySize, 
                            illegalPortal = nil, 
                            arrivalBasis = nil, 
                            timeDirection = object.boxCarryDirection}, 
                        targetFrame = self.frameNumber+timeDirectionToInt(object.boxCarryDirection),
                    }
                end
                if object.type == "box" then
                    object.y = object.y - 12000
                end
                return object
            end,
        }
    },
    protoButtons = {
        {
            attachment = {platform = 1, xOffset = 3200, yOffset = -800},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 1
        },
        {
            attachment = {platform = nil, xOffset = 3200, yOffset = 37600},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 3
        }
    }
}