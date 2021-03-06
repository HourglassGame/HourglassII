name = "ThePit"
speedOfTime = 1
timelineLength = 7200
environment =
{
    gravity = 10,
    wall =
    {
        width = 18,
        height = 12,
        segmentSize = 3200,
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1},
        {1,1,1,0,0,0,1,1,0,0,0,1,1,1,0,0,0,1},
        {1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1},
        {1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,1},
        {1,1,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    }
}
initialGuy =
{
    arrivalTime = 0,
    arrival =
    {
        x = 1.75 * 3200,
        y = 4 * 3200,
        width = 1600,
        height = 3200,
        facing = "right",
        timeDirection = "forwards",
        pickups = {timeJump = 0, timeReverse = 0, timeGun = 0},
    }
}
initialArrivals =
{
    {type = "box", x = 12.75 * 3200, y = 7 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
}
triggerSystem =
{
    type = "DirectLua",

    luaFiles = {"triggerSystem", "global.basicTriggerSystem"},
    system = [==[require "triggerSystem"]==],

    -- C++ still manages offsets and defaults.
    triggerOffsetsAndDefaults = {
        {
            offset = -600,
            default = {0}
        },
        {
            offset = -900,
            default = {0}
        },
        {
            offset = 1,
            default = {0}
        },
        {
            offset = 1,
            default = {8 * 3200, 5 * 3200, 0, 0}
        },
        {
            offset = 1,
            default = {1}
        },
        {
            offset = 1,
            default = {1}
        },
    },
    arrivalLocationsSize = 1,
}