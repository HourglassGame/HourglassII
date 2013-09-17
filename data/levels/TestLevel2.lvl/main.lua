name = "Going Up"
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
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    }
}
initialGuy =
{
    arrivalTime = 0,
    arrival =
    {
        x = 20 * 3200,
        y = 17 * 3200,
        width = 1600,
        height = 3200,
        facing = "right",
        timeDirection = "forwards",
        pickups = {timeJump = -1, timeReverse = -1, timeGun = -1, timePause = -1},
		boxCarrying = true,
        boxCarrySize = 3200,
        boxCarryDirection = "forwards",
    }
}
initialArrivals =
{
	{type = "box", x = 9*3200, y = 14 * 3200 , xspeed = 0,     yspeed = 0, 	size = 3200, timeDirection = "forwards"},
    --{type = "box", x = 32400, y = 8000,  xspeed = 0, 	 yspeed = -600, size = 3200, timeDirection = "forwards"},
    --{type = "box", x = 46400, y = 14200, xspeed = -1000, yspeed = -500, size = 3200, timeDirection = "forwards"},
    --{type = "box", x = 46400, y = 10800, xspeed = -1000, yspeed = -500, size = 3200, timeDirection = "forwards"},
    --{type = "box", x = 46400, y = 17600, xspeed = -1000, yspeed = -500, size = 3200, timeDirection = "forwards"},
    --{type = "box", x = 46400, y = 21600, xspeed = -500,  yspeed = -500, size = 3200, timeDirection = "forwards"},
    --{type = "box", x = 6400,  y = 15600, xspeed = 1000,  yspeed = -500, size = 3200, timeDirection = "forwards"},
    --{type = "box", x = 56400, y = 15600, xspeed = 0,     yspeed = 0, 	size = 3200, timeDirection = "forwards"}
}
triggerSystem =
{
    type = "DirectLua",
    
    luaFiles = {"triggerSystem", "global.basicTriggerSystem"},
    system = [==[ require "triggerSystem" ]==],

    -- C++ still manages offsets and defaults.
    triggerOffsetsAndDefaults = {
        {
            offset=1,
            default={0}
        },
        {
            offset = 1,
            default = {15 * 3200, 18 * 3200, 0, 0}
        },
        {
            offset = 1,
            default = {0}
        },
        {
            offset = 1,
            default = {21 * 3200, 16 * 3200, 0, 0}
        },
    },
    arrivalLocationsSize = 2
}