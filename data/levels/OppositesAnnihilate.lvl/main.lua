name = "Opposites Annihilate"
speedOfTime = 24
timelineLength = 5401 -- Important, improves loading time.
environment =
{
    gravity = 10,
    wall =
    {
        width = 18,
        height = 11,
        segmentSize = 3200,
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
        {1,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,1,1},
        {1,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    }
}
initialGuy =
{
    arrivalTime = 0,
    arrival =
    {
        x = 10.5 * 3200,
        y = 8 * 3200,
        width = 1600,
        height = 3200,
        facing = "left",
        timeDirection = "forwards",
        pickups = {timeJump = 0, timeReverse = 0, timeGun = 0},
    }
}
initialArrivals =
{
    {type = "box", x = 4.5 * 3200, y = 6 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
    {type = "box", x = 4.5 * 3200, y = 6 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "reverse"},
}
triggerSystem =
{
    type = "ConfiguredLua",

    luaFiles = {"triggerSystem", "global.basicTriggerSystem"},
    system = [==[require "triggerSystem"]==],

    -- C++ still manages offsets and defaults.
    triggerOffsetsAndDefaults = {
    },
    arrivalLocationsSize = 1,
}