name = "Easy Start"
speedOfTime = 3
timelineLength = 10800
environment = 
{
    gravity = 30,
    wall = 
    {
        width = 32,
        height = 19,
        segmentSize = 3200,
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
        {1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
        {1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
        {1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    }
}
initialGuy = 
{
    arrivalTime = 0,
    arrival = 
    {
        x = 11 * 3200,
        y = 11 * 3200,
        width = 1600,
        height = 3200,
        facing = "right",
        timeDirection = "forwards",
    }
}
triggerSystem =
{
    type = "DirectLuaTriggerSystem",
    system =

        [===[
--the socialist alternative:
--get("HGUtil.include")(triggerSystem.lua)
--another win for capitalism:
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
            local function sign(a) return get("math.abs")(a) / a end
            
            local desiredPosition = destination.desiredPosition
            local deceleration = destination.deceleration
            local acceleration = destination.acceleration
            if desiredPosition ~= position then
                if 
                    (get("math.abs")(desiredPosition - position) <= get("math.abs")(velocity)) 
                        and
                    (get("math.abs")(velocity) <= deceleration)
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
                        get("math.abs")(position - desiredPosition)
							>
                        (square(velocity - direction * acceleration) * 3 / (2 * deceleration))
                    then
                        velocity = velocity - direction * acceleration
                    elseif get("math.abs")(position - desiredPosition) > (square(velocity)*3/(2*deceleration))
                    then
                        velocity =
                            -direction * get("math.sqrt")(
                                get("math.abs")(position - desiredPosition)
                                * deceleration * 2 / 3)
                    else
                        velocity = velocity + direction * deceleration
                    end
                end
            else
            	if get("math.abs")(velocity) <= deceleration then
				    velocity = 0
			    else
				    velocity = velocity + (get("math.abs")(velocity)/velocity)*deceleration
			    end
            end
            local maxSpeed = destination.maxSpeed
            if get("math.abs")(velocity) > maxSpeed then
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
    for i, protoCollision in get("ipairs")(protoCollisions) do
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
    for i, protoPortal in get("ipairs")(protoPortals) do
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
    for i, portal in get("ipairs")(portals) do
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
    for i, protoButton in get("ipairs")(protoButtons) do
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
        for frame, objectList in get("pairs")(departures) do
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

local tempStore = 
{
    --mutable store data:
    buttonPositionsAndVelocities = {},
    outputTriggers = {},
    outputGlitz = {},
    
    --constant proto-object data:
    protoPortals =
    {
        {
            x = 7 * 3200,
            y = 11 * 3200,
            index = 1,
            width = 2 * 3200,
            height = 2 * 3200,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = 1,
            xDestination = 0,
            yDestination = 0,
            relativeTime = true,
            timeDestination = -8 * 60,
            illegalDestination = 1,
            fallable = false,
            winner = false
        },
        {
            x = 45 * 1600,
            y = 11 * 3200,
            index = 2,
            width = 2 * 3200,
            height = 2 * 3200,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = 2,
            xDestination = 0,
            yDestination = 0,
            relativeTime = true,
            timeDestination = -8 * 60,
            illegalDestination = 1,
            fallable = false,
            winner = true
        }
    },
    protoCollisions = {
        {
            width = 3200,
            height = 4 * 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 2,
            buttonTriggerID = 1,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 41 * 1600,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 8 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 41 * 1600,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 4 * 3200,
                        maxSpeed = 300,
                        acceleration = 20,
                        deceleration = 20
                    }
                }
            }
        }
    },
    protoMutators = {
    },
    protoButtons = {
        {
            attachment = {platform = nil, xOffset = 27 * 1600, yOffset = 11 * 3200},
            width = 3200,
            height = 800,
            timeDirection = 'forwards',
            triggerID = 1
        }
    }
}
--==Callin Definitions==--
--triggerArrivals have already had default values inserted by C++
--for trigger indices that did not arrive by the time this is called
function calculatePhysicsAffectingStuff(frameNumber, triggerArrivals)
    local retv = {}
    
    tempStore.frameNumber = frameNumber
    
    retv.additionalBoxes = {}
    tempStore.additionalEndBoxes = {}

    retv.collisions = calculateCollisions(tempStore.protoCollisions, triggerArrivals)
    retv.portals = calculatePortals(tempStore.protoPortals, retv.collisions)
    retv.mutators = { [1] = tempStore.protoMutators[1].data}
    retv.arrivalLocations = calculateArrivalLocations(retv.portals)
    
    tempStore.buttonPositionsAndVelocities =
        calculateButtonPositionsAndVelocities(tempStore.protoButtons, retv.collisions)
     
    fillCollisionTriggers(tempStore.outputTriggers, tempStore.protoCollisions, retv.collisions)
    
    for collision in list_iter(retv.collisions) do
        get("table.insert")(tempStore.outputGlitz, calculateCollisionGlitz(collision))
    end
    
    for portal in list_iter(retv.portals) do
        get("table.insert")(tempStore.outputGlitz, calculatePortalGlitz(portal))
    end
    
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
    return true
end
function mutateObject(responsibleManipulatorIndices, dynamicObject)
    return dynamicObject
end
    
function getDepartureInformation(departures)
    local buttonStates =
        calculateButtonStates(tempStore.protoButtons, tempStore.buttonPositionsAndVelocities, departures)

    for i = 1, #tempStore.protoButtons do
        get("table.insert")(
            tempStore.outputGlitz,
            calculateButtonGlitz(
                tempStore.protoButtons[i],
                tempStore.buttonPositionsAndVelocities[i],
                buttonStates[i]))
    end
    
    get("table.insert")(tempStore.outputGlitz, calculateMutatorGlitz(tempStore.protoMutators[1].data))
    
    fillButtonTriggers(tempStore.outputTriggers, tempStore.protoButtons, buttonStates)
    
    return tempStore.outputTriggers, tempStore.outputGlitz, {}, tempStore.additionalEndBoxes
end
    ]===],


    -- C++ still manages offsets and defaults.
    triggerOffsetsAndDefaults = {
        {
            offset = 1,
            default = {0}
        },
        {
            offset = 1,
            default = {38400, 43800, 0, 0}
        }
    },
    arrivalLocationsSize = 1   
}


--[=[
{
    type = "HourglassI"
    --Should this allow reverseTime movers/collisions/switches
    --or would that introduce too much complexity?
    
    --Somehow put magical glitz on portals, switches and collisions?
    portals = {
        {
            -- -8 portal
            x = 7 * 3200,
            y = 11 * 3200,
            width = 2 * 3200,
            height = 2 * 3200,
            relative = true,
            destination = -8 * 60
        },
        {
            -- win portal
            x = 45 * 1600,
            y = 11 * 3200,
            width = 2 * 3200,
            height = 2 * 3200,
            winner = true
        }
    },
    collisions = {
        {
            -- Door attached to mover 1
            mover = 1,
            x = 0,
            y = 0,
            width = 3200,
            height = 4 * 3200
        }
    },
    movers = {
        {
            -- Mover activated by button 1
            switcher = {"trigger", 1}
            on = {
                x = 41 * 1600,
                y = 8 * 3200
            }
            off = {
                x = 41 * 1600,
                y = 4 * 3200
            }
        }
    },
    buttons = {
        {
            -- Momentary action button on the floor
            action = "momentary",
            x = 27 * 1600,
            y = 11 * 3200,
            width = 3200,
            height = 800
        }
    },
    triggers {
        {
            offset = 1,
            default = {},
            
        }
    }--[[,
    glitz = {
        --Somehow do background and foreground "magically"
        {
            --background
        },
        {
            --foreground
        },
        --Allow control substitutions (eg "press <insert jump control here> to jump")
        --and translations.
        {
            --tutorial text 1
        },
        {
            --tutorial text 2
        }
    }]]--
}]=]--

