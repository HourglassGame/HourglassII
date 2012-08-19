local triggerSystemString = [===[
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
            velocity = velocity >= 0 and get("math.floor")(velocity) or get("math.ceil")(velocity)
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
    buttonPositionsAndVelocities = {}
    for i, protoButton in get("ipairs")(protoButtons) do
        protoButton:calcPnV(collisions)
    end
end

local function calculateButtonStates(protoButtons, departures, triggerArrivals)
    for i = 1, #protoButtons do
        protoButtons[i]:updateState(departures, triggerArrivals)
    end
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

local function cloneAttachment(a)
    return {platform = a.platform, xOffset = a.xOffset, yOffset = a.yOffset}
end

local function checkPressed(buttonArea, departures)
    for frame, objectList in get("pairs")(departures) do
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
    return
    {
        x = buttonPositionAndVelocity.x,
        y = buttonPositionAndVelocity.y,
        width = proto.width,
        height = proto.height,
        xspeed = buttonPositionAndVelocity.xspeed,
        yspeed = buttonPositionAndVelocity.yspeed,
        forwardsColour = colour,
        reverseColour = colour,
        timeDirection = proto.timeDirection
    }
end

local function momentarySwitch(p)
    local PnV = nil
    local state = nil
    
    local triggerID = p.triggerID
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
            state = checkPressed(constructDynamicArea(proto, PnV), departures)
        end,
        calculateGlitz = function(self, outputGlitz)
            get("table.insert")(outputGlitz, calculateButtonGlitz(proto, PnV, state))
        end,
        fillTrigger = function(self, outputTriggers)
            outputTriggers[triggerID] = {state and 1 or 0}
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
        calculateGlitz = function(self, outputGlitz)
            get("table.insert")(outputGlitz, calculateButtonGlitz(constructCompleteProto(self.timeDirection, self.first), firstPnV, switchState == 1))
            get("table.insert")(outputGlitz, calculateButtonGlitz(constructCompleteProto(self.timeDirection, self.second), secondPnV, switchState == 0))
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

local tempStore =
{
    --mutable store data:
    buttonPositionsAndVelocities = {},
    outputTriggers = {},
    outputGlitz = {},
    triggerArrivals = nil,

    --constant proto-object data:
    protoPortals =
    {
        {
            attachment = {platform = nil, xOffset = 15 * 3200, yOffset = 16 * 3200},
            index = 1,
            width = 2 * 3200,
            height = 2 * 3200,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = 1,
            xDestination = 0,
            yDestination = 0,
            relativeTime = false,
            timeDestination = 0,
            illegalDestination = 1,
            fallable = false,
            winner = false
        },
        {
            attachment = {platform = nil, xOffset = 22 * 3200, yOffset = 16 * 3200},
            index = 2,
            width = 2 * 3200,
            height = 2 * 3200,
            collisionOverlap = 50,
            timeDirection = 'forwards',
            destinationIndex = 2,
            xDestination = 0,
            yDestination = 0,
            relativeTime = false,
            timeDestination = 0,
            illegalDestination = 2,
            fallable = false,
            winner = true
        }
    },
    protoCollisions = {
        {
            width = 3*3200,
            height = 3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 2,
            buttonTriggerID = 1,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 18 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 3 * 3200,
                        maxSpeed = 300,
                        acceleration = 20,
                        deceleration = 20
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 18 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 18 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                }
            }
        },
        {
            width = 3200,
            height = 2*3200,
            timeDirection = 'forwards',
            lastStateTriggerID = 4,
            buttonTriggerID = 3,
            destinations =
            {
                onDestination = {
                    xDestination = {
                        desiredPosition = 21 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 14 * 3200,
                        maxSpeed = 300,
                        acceleration = 20,
                        deceleration = 20
                    }
                },
                offDestination = {
                    xDestination = {
                        desiredPosition = 21 * 3200,
                        maxSpeed = 200,
                        acceleration = 50,
                        deceleration = 50
                    },
                    yDestination = {
                        desiredPosition = 16 * 3200,
                        maxSpeed = 300,
                        acceleration = 50,
                        deceleration = 50
                    }
                }
            }
        },
    },
    protoMutators = {
    },
    protoButtons = {
        toggleSwitch{
            triggerID = 1,
            timeDirection = 'forwards',
            first = {
                attachment = {platform = nil, xOffset = 10 * 3200, yOffset = 17 * 3200},
                width = 800,
                height = 1600,
            },
            second = {
                attachment = {platform = nil, xOffset = 23 * 3200, yOffset = 4 * 3200},
                width = 800,
                height = 1600,
            }
        },
        momentarySwitch{
            triggerID = 3,
            timeDirection = 'forwards',
            attachment = {platform = nil, xOffset = 29*3200 - 800, yOffset = 4 * 3200 },
            width = 800,
            height = 3200,
        },
    }
}
--==Callin Definitions==--
--triggerArrivals have already had default values inserted by C++
--for trigger indices that did not arrive by the time this is called
function calculatePhysicsAffectingStuff(frameNumber, triggerArrivals)
    local retv = {}

    tempStore.frameNumber = frameNumber
    tempStore.triggerArrivals = triggerArrivals

    retv.additionalBoxes = {}
    tempStore.additionalEndBoxes = {}

    retv.collisions = calculateCollisions(tempStore.protoCollisions, triggerArrivals)
    retv.portals = calculatePortals(tempStore.protoPortals, retv.collisions)
    retv.mutators = {}
    retv.arrivalLocations = calculateArrivalLocations(retv.portals)

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
    calculateButtonStates(tempStore.protoButtons, departures, tempStore.triggerArrivals)

    for i = 1, #tempStore.protoButtons do
        tempStore.protoButtons[i]:calculateGlitz(tempStore.outputGlitz)
    end

    for i = 1, #tempStore.protoButtons do
        tempStore.protoButtons[i]:fillTrigger(tempStore.outputTriggers)
    end
    
    return tempStore.outputTriggers, tempStore.outputGlitz, {}, tempStore.additionalEndBoxes
end
    ]===]


name = "Going Up"
speedOfTime = 3
timelineLength = 10800
environment =
{
    gravity = 22,
    wall =
    {
        width = 32,
        height = 19,
        segmentSize = 3200,
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,0,0,0,0,0,0,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,0,0,0,0,0,0,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    }
}
initialGuy =
{
    arrivalTime = 0,
    arrival =
    {
        x = 29 * 1600,
        y = 17 * 3200,
        width = 1600,
        height = 3200,
        facing = "right",
        timeDirection = "forwards",
        pickups = {timeJump = 0, timeReverse = 0, timeGun = 0}
    }
}
initialArrivals =
{
}
triggerSystem =
{
    type = "DirectLua",
    system = triggerSystemString,

    -- C++ still manages offsets and defaults.
    triggerOffsetsAndDefaults = {
        {
            offset=1,
            default={0}
        },
        {
            offset = 1,
            default = {18 * 3200, 18 * 3200, 0, 0}
        },
        {
            offset = 1,
            default = {0}
        },
        {
            offset = 1,
            default = {21 * 3200, 16 * 3200, 0, 0}
        },
    },
    arrivalLocationsSize = 2
}