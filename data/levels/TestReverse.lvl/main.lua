name = "Test Reverse"
speedOfTime = 1
timelineLength = 3600
environment =
{
    gravity = 22,
    wall =
    {
        width = 25,
        height = 17,
        segmentSize = 3200,
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    }
}
initialGuy =
{
    arrivalTime = 1800,
    arrival =
    {
        x = 5.5 * 3200,
        y = 10 * 3200,
        width = 1600,
        height = 3200,
        facing = "right",
        timeDirection = "forwards",
        pickups = {timeJump = -1, timeReverse = -1, timeGun = 0},
    }
}
initialArrivals =
{
    --{type = "box", x = 5.8 * 3200, y = 2 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
    --{type = "box", x = 5 * 3200, y = 10 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
    --
    --{type = "box", x = 10.9 * 3200, y = 2 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
    --{type = "box", x = 10 * 3200, y = 10 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "reverse"},
    --
    --{type = "box", x = 12.8 * 3200, y = 2 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
    --{type = "box", x = 12 * 3200, y = 10 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "reverse"},
    --
    --{type = "box", x = 14.7 * 3200, y = 2 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
    --{type = "box", x = 14 * 3200, y = 10 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "reverse"},
    --
    --{type = "box", x = 16.6 * 3200, y = 2 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
    --{type = "box", x = 16 * 3200, y = 10 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "reverse"},
    
    {type = "box", x = 20 * 3200, y = 14 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
    {type = "box", x = 15 * 3200, y = 14 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
    {type = "box", x = 11 * 3200, y = 5 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "reverse"},
    {type = "box", x = 9 * 3200, y = 5 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
    {type = "box", x = 7 * 3200, y = 5 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "reverse"},
}
triggerSystem =
{
    type = "DirectLua",

    luaFiles = {"triggerSystem", "global.basicTriggerSystem"},
    system = [==[require "triggerSystem"]==],

    -- C++ still manages offsets and defaults.
    triggerOffsetsAndDefaults = {
        {
            offset = 1,
            default = {0}
        },
        {
            offset = 1,
            default = {10 * 3200, 15 * 3200, 0, 0}
        },
    },
    arrivalLocationsSize = 1,
}