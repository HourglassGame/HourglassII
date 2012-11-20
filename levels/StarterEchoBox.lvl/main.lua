--========================================
--Example HourglassII level file.
--This file is intended to demonstrate 
--and partially document all of the
--lua level interface.
--========================================

--When a level is loaded the script for the level is run.
--The script must set the values of specific global
--variables which are required to describe a level.

--Precise documentation of what those fields actually are
--will come later, but for now this level should demonstrate
--all the supported features.

--All of the global fields that filled in by this file are compulsory
--(must be in every level)

--====   Further Development ====--
--Glitz is tricky, it is packaged with a level within the
--same folder as the level script. The rest is not yet designed.

--My current working model would restrict glitz to only stuff that
--had hard-coded semantics, and would not allow (for example)
--dynamic generation and definition of glitz.

--This is probably ok.
speedOfTime = 3
timelineLength = 10800
environment = 
{
    gravity = 25,
    wall = 
    {
        width = 20,
        height = 15,
        segmentSize = 3200,
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
    }
}
initialGuy = 
{
    arrivalTime = 0,
    arrival = 
    {
        x = 6400,
        y = 41600,
        xspeed = 0,
        yspeed = 0,
        width = 1600,
        height = 3200,
        pickups = {timeJump = 0, timeReverse = 0},
        facing = "right",
        boxCarrying = false,
        --these two fields are only needed
        --if boxCarrying is true.
        --boxCarrySize = 0,
        --boxCarryDirection = INVALID,
        timeDirection = "forwards",
    }
}
--only boxes for now, but the plan is that this is all dynamic objects other than guys
initialArrivals =
{
    {type = "box", x = 19200, y = 41600, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
	{type = "box", x = 38400, y = 32000, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
}
triggerSystem = 
{
    --DirectLua does a minimum amount of work in C++, before calling
    --Lua with the same arguments and expecting the same return values
    --as the TriggerSystem in C++
    type = "DirectLua",
    
    luaFiles = {"triggerSystem", "global.basicTriggerSystem"},
    --This describes a lua chunk which will be loaded by TriggerFrameState
    --to do its stuff.
    system = [==[require "triggerSystem"]==],

    -- C++ still manages offsets and defaults.
    triggerOffsetsAndDefaults = {
    },
    arrivalLocationsSize = 3
}

