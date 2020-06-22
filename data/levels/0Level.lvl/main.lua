name = "Guy Killer"
speedOfTime = 1 --24
timelineLength = 5000
environment =
{
    gravity = 10,
    wall =
    {
        width = 25,
        height = 17,
        segmentSize = 3200,
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1},
        {1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    }
}
initialGuy =
{
    arrivalTime = 0,
    arrival =
    {
        x = 7.2 * 3200,
        y = 9 * 3200,
        width = 1600,
        height = 3200,
        facing = "right",
		boxCarrying = "balloon",
		boxCarryWidth = 1000*1.33,
		boxCarryHeight = 3000*1.33,
		boxCarryDirection = "forwards",
        timeDirection = "forwards",
        pickups = {timeJump = 1000, timeReverse = 1000, timeGun = 1000},
    }
}
initialArrivals =
{
    --{type = "box", x = 5.8 * 3200, y = 2 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
    --{type = "box", x = 5 * 3200, y = 10 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
    --
    --{type = "box", x = 10.9 * 3200, y = 2 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
    --{type = "box", x = 10 * 3200, y = 10 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "reverse"},
    --
    --{type = "box", x = 12.8 * 3200, y = 2 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
    --{type = "box", x = 12 * 3200, y = 10 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "reverse"},
    --
    --{type = "box", x = 14.7 * 3200, y = 2 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
    --{type = "box", x = 14 * 3200, y = 10 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "reverse"},
    --
    --{type = "box", x = 16.6 * 3200, y = 2 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
    --{type = "box", x = 16 * 3200, y = 10 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "reverse"},

    {type = "box", boxType = "balloon", x = 8 * 3200, y = 8 * 3200, xspeed = 0, yspeed = 0, width = 1000, height = 3000, timeDirection = "forwards"},
    {type = "box", boxType = "balloon", x = 9 * 3200, y = 8 * 3200, xspeed = 0, yspeed = 0, width = 1000*1.33, height = 3000*1.33, timeDirection = "forwards"},
    {type = "box", boxType = "balloon", x = 10 * 3200, y = 8 * 3200, xspeed = 0, yspeed = 0, width = 1000*1.66, height = 3000*1.66, timeDirection = "forwards"},
    {type = "box", boxType = "balloon", x = 11 * 3200, y = 8 * 3200, xspeed = 0, yspeed = 0, width = 1000*2, height = 3000*2, timeDirection = "forwards"},
    {type = "box", boxType = "balloon", x = 12.2 * 3200, y = 8 * 3200, xspeed = 0, yspeed = 0, width = 1000*2.33, height = 3000*2.33, timeDirection = "forwards"},
    {type = "box", boxType = "balloon", x = 14 * 3200, y = 8 * 3200, xspeed = 0, yspeed = 0, width = 1000*2.66, height = 3000*2.66, timeDirection = "forwards"},
    {type = "box", boxType = "balloon", x = 16 * 3200, y = 13 * 3200, xspeed = 0, yspeed = 0, width = 1000*3, height = 3000*3, timeDirection = "forwards"},
	
    {type = "box", x = 4 * 3200, y = 7 * 3200, xspeed = 0, yspeed = 0, width = 3200, height = 3200, timeDirection = "forwards"},
    {type = "box", x = 4 * 3200, y = 8 * 3200, xspeed = 0, yspeed = 0, width = 3200, height = 3200, timeDirection = "forwards"},
    {type = "box", x = 4 * 3200, y = 9 * 3200, xspeed = 0, yspeed = 0, width = 3200, height = 3200, timeDirection = "forwards"},
    {type = "box", x = 4 * 3200, y = 10 * 3200, xspeed = 0, yspeed = 0, width = 3200, height = 3200, timeDirection = "forwards"},
    
    {type = "box", x = 13.9 * 3200, y = 2 * 3200, xspeed = 0, yspeed = 0, width = 3000, height = 6400, timeDirection = "forwards"},
    {type = "box", x = 9.5 * 3200, y = 5 * 3200, xspeed = 0, yspeed = 0, width = 1600, height = 2400, timeDirection = "forwards"},
    {type = "box", boxType = "balloon", x = 11 * 3200, y = 3 * 3200, xspeed = 0, yspeed = 0, width = 1000, height = 3000, timeDirection = "forwards"},
    --{type = "box", x = 10 * 3200, y = 5 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "reverse"},
	
    {type = "box", x = 11 * 3200, y = 8.5 * 3200, xspeed = 0, yspeed = 0, width = 8700, height = 2100, timeDirection = "forwards"},
    {type = "box", x = 10 * 3200, y = 10 * 3200, xspeed = 0, yspeed = 0, width = 8700, height = 2100, timeDirection = "forwards"},
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
            default = {12 * 3200, 12 * 3200, 0, 0}
        },
    },
    arrivalLocationsSize = 1,
}