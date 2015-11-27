name = "Wacky Tomb"
speedOfTime = 3
timelineLength = 8000
environment =
{
    gravity = 22,
    wall =
    {
        width = 24,
        height = 15,
        segmentSize = 3200,
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1},
        {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
        {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
        {1,1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    }
}
initialGuy =
{
    arrivalTime = 4000,
    arrival =
    {
        x = 3.5 * 3200,
        y = 5 * 3200,
        width = 1600,
        height = 3200,
        facing = "right",
        timeDirection = "reverse",
        pickups = {timeJump = 0, timeReverse = 0, timeGun = 0},
    }
}
initialArrivals =
{
    {type = "box", x = 14 * 3200, y = 5 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
}
triggerSystem =
{
    type = "DirectLua",

    luaFiles = {"triggerSystem", "global.basicTriggerSystem"},
    system = [==[require "triggerSystem"]==],

    -- C++ still manages offsets and defaults.
    triggerOffsetsAndDefaults = {
        {
            offset=1,
            default={1}
        },
        {
            offset=1,
            default={1}
        },
        {
            offset = 1,
            default = {0}
        },
        {
            offset = 1,
            default = {0}
        },
        {
            offset = 1,
            default = {0}
        },
        {
            offset = 1,
            default = {17 * 3200, 3 * 3200, 0, 0}
        },
    },
    arrivalLocationsSize = 1,
}