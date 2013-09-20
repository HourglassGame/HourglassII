name = "Wait A Moment"
speedOfTime = 3
timelineLength =5400
environment =
{
    gravity = 22,
    wall =
    {
		width = 16,
        height = 9,
        segmentSize = 3200,
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    }
}
initialGuy =
{
    arrivalTime = 0,
    arrival =
    {
        x = 6 * 3200,
        y = 5 * 3200,
        width = 1600,
        height = 3200,
        facing = "right",
        timeDirection = "forwards",
        pickups = {timePause = 0},
    }
}
initialArrivals =
{
    {type = "box", x = 12 * 3200, y = 7 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"}
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
    },
    arrivalLocationsSize = 2,
}