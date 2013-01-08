name = "Elevator"
speedOfTime = 3
timelineLength = 6000
environment =
{
    gravity = 25,
    wall =
    {
        width = 15,
        height = 14,
        segmentSize = 3200,
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
        {1,1,1,1,1,1,1,1,1,1,0,0,0,1,1},
        {1,1,1,1,1,1,1,1,1,1,0,0,0,1,1},
        {1,1,1,1,1,1,1,1,1,1,0,0,0,1,1},
        {1,1,1,1,1,1,1,1,1,1,0,0,0,1,1},
        {1,1,1,1,1,1,1,1,1,1,0,0,0,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
        {1,1,1,1,1,1,1,1,1,1,0,0,0,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    }
}
initialGuy =
{
    arrivalTime = 0,
    arrival =
    {
        x = 4 * 3200,
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
	{type = "box", x = 7 * 3200, y = 3 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
}
triggerSystem =
{
    type = "DirectLua",
    
    luaFiles = {"triggerSystem", "global.basicTriggerSystem"},
    system = [==[ require "triggerSystem" ]==],

    -- C++ still manages offsets and defaults.
    triggerOffsetsAndDefaults = {
        {
            offset = -600,
            default = {0}
        },
        {
            offset = 1,
            default = {10 * 3200, 12 * 3200, 0, 0}
        },
    },
    arrivalLocationsSize = 1
}