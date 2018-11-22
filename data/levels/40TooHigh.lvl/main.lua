name = "Too High"
speedOfTime = 1
timelineLength = 10800
environment =
{
    gravity = 22,
    wall =
    {
        width = 14,
        height = 9,
        segmentSize = 3200,
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    }
}
initialGuy =
{
    arrivalTime = 0,
    arrival =
    {
        x = 2 * 3200,
        y = 7 * 3200,
        width = 1600,
        height = 3200,
        facing = "right",
        timeDirection = "forwards",
        pickups = {timeJump = 0, timeReverse = 0, timeGun = 0}
    }
}
initialArrivals =
{
    {type = "box", x = 5 * 3200, y = 7 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
}
triggerSystem =
{
    type = "CompatibleLua",

    luaFiles = {"triggerSystem", "global.basicTriggerSystem"},
    system = [==[ require "triggerSystem" ]==],


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
    },
    arrivalLocationsSize = 1
}