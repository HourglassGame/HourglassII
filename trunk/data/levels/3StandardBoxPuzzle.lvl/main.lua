name = "Standard Box Puzzle"
speedOfTime = 3
timelineLength = 10800
environment =
{
    gravity = 25,
    wall =
    {
        width = 32,
        height = 19,
        segmentSize = 3200,
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1},
        {1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
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
        x = 19 * 3200,
        y = 11 * 3200,
        width = 1600,
        height = 3200,
        facing = "right",
        timeDirection = "forwards",
        pickups = {timeJump = 0, timeReverse = 0, timeGun = 0}
    }
}
initialArrivals =
{
    {type = "box", x = 13 * 3200, y = 11 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
    {type = "box", x = 15 * 3200, y = 11 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"}
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
            default = {8 * 3200, 9 * 3200, 0, 0}
        }
    },
    arrivalLocationsSize = 2
}