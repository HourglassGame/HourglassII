name = "Easy Start"
speedOfTime = 1
timelineLength = 10800
environment = 
{
    gravity = 22,
    wall = 
    {
        width = 12,
        height = 12,
        segmentSize = 3200,
        {1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1},
    }
}
initialGuy = 
{
    arrivalTime = 0,
    arrival = 
    {
        x = 5 * 3200,
        y = 5 * 3200,
        width = 1600,
        height = 3200,
        facing = "right",
        timeDirection = "forwards",
        pickups = {timeJump = -1, timeReverse = -1, timeGun = -1, timePause = -1}
    }
}
initialArrivals =
{
}
for x = 1, 9 do
    for y = 2, 10 do
        initialArrivals[#initialArrivals+1] = {type = "box", x = x * 3200, y = y * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"}
    end
end
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
    },
    arrivalLocationsSize = 2
}
