name = "HoldThemAll"
speedOfTime = 3
timelineLength = 8000
environment =
{
    gravity = 22,
    wall =
    {
        width = 22,
        height = 8,
        segmentSize = 3200,
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1},
        {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    }
}
initialGuy =
{
    arrivalTime = 4000,
    arrival =
    {
        x = 1.5 * 3200,
        y = 6 * 3200,
        width = 1600,
        height = 3200,
        facing = "right",
        timeDirection = "reverse",
        pickups = {timeJump = 0, timeReverse = 1, timeGun = 0, timePause = 0},
		boxCarrying = true,
        boxCarrySize = 3200,
        boxCarryDirection = "reverse",
    }
}
initialArrivals =
{
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
            default = {16 * 3200, 3 * 3200, 0, 0}
        }
    },
    arrivalLocationsSize = 1,
}