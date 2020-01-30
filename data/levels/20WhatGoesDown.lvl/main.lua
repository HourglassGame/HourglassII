name = "What goes down..."
speedOfTime = 24
timelineLength = 5000
environment =
{
    gravity = 10,
    wall =
    {
        width = 25,
        height = 14,
        segmentSize = 3200,
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,0,0,0,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,1},
        {1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,1},
        {1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1,0,0,1},
        {1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,0,0,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    }
}
initialGuy =
{
    arrivalTime = 0,
    arrival =
    {
        x = 19.5 * 3200,
        y = 4 * 3200,
        width = 1600,
        height = 3200,
        facing = "left",
        timeDirection = "forwards",
        pickups = {timeJump = 0, timeReverse = 0, timeGun = 0},
    }
}
initialArrivals =
{
    {type = "box", x = 16 * 3200, y = 4 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
}
triggerSystem =
{
    type = "ConfiguredLua",

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
            default = {22 * 3200, 5.25 * 3200, 0, 0}
        },
        {
            offset = 1,
            default = {20 * 3200, 10 * 3200, 0, 0}
        }
    },
    arrivalLocationsSize = 1,
}