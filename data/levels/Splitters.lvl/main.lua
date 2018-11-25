name = "Splitters"
speedOfTime = 1
timelineLength = 5400
environment =
{
    gravity = 10,
    wall =
    {
        width = 21,
        height = 12,
        segmentSize = 3200,
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1},
        {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
        {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
        {1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    }
}
initialGuy =
{
    arrivalTime = 300,
    arrival =
    {
        x = 10 * 1600,
        y = 6 * 3200,
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
    
    luaFiles = {"triggerSystem", "global.basicTriggerSystem"},
    system = [==[ require "triggerSystem" ]==],

    -- C++ still manages offsets and defaults.
    triggerOffsetsAndDefaults = {
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
            default = {9 * 3200, 4 * 3200, 0, 0}
        },
        {
            offset = 1,
            default = {1}
        },
        {
            offset = 1,
            default = {1}
        },
        {
            offset = -300,
            default = {0}
        },
        {
            offset = 1,
            default = {8.5 * 3200, 8 * 3200, 0, 0}
        },
        {
            offset = 1,
            default = {12 * 3200, 8 * 3200, 0, 0}
        },
        {
            offset = 1,
            default = {0}
        },
        {
            offset = 1,
            default = {0,0}
        }
    },
    arrivalLocationsSize = 2
}