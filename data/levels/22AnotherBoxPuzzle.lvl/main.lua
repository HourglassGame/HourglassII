name = "Another Box Puzzle"
speedOfTime = 3
timelineLength = 6000
environment = 
{
    gravity = 22,
    wall = 
    {
        width = 26,
        height = 14,
        segmentSize = 3200,
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    }
}
initialGuy = 
{
    arrivalTime = 0,
    arrival = 
    {
        x = 3.5 * 3200,
        y = 9 * 3200,
        width = 1600,
        height = 3200,
        facing = "right",
        timeDirection = "forwards",
        pickups = {timeJump = 0, timeReverse = 0, timeGun = 0}
    }
}
initialArrivals =
{
    {type = "box", x = 1.5 * 3200, y = 8 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"}
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
    triggerOffsetsAndDefaults = {
        {
            offset = 1,
            default = {1}
        },
        {
            offset = 1,
            default = {1}
        },
    },
    arrivalLocationsSize = 1
}
