name = "Standard Balloons"
speedOfTime = 24
speedOfTimeFuture = 0
timelineLength = 10800
environment =
{
	gravity = 10,
	wall =
	{
		width = 32,
		height = 19,
		segmentSize = 3200,
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,0,0,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,0,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,0,1,1,1,1},
		{1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,0,1,1,1,1},
		{1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,0,1,1,0,1,1,1,1},
		{1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1},
		{1,0,0,0,0,0,0,0,0,0,1,1,0,1,1,0,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,1,1,0,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,1,1,0,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	}
}
initialGuy =
{
	arrivalTime = 0,
	arrival =
	{
		x = 14.75 * 3200,
		y = 15 * 3200,
		width = 1600,
		height = 3200,
		facing = "right",
		timeDirection = "forwards",
		pickups = {timeJump = 0, timeReverse = 0, timeGun = 0}
	}
}
initialArrivals =
{
	{type = "box", boxType = "balloon", x = 4 * 3200, y = 14 * 3200, xspeed = 0, yspeed = 0, width = 1 * 1600, height = 1 * 3200, timeDirection = "forwards"},
	{type = "box", boxType = "balloon", x = 7 * 3200, y = 14 * 3200, xspeed = 0, yspeed = 0, width = 1.3 * 1600, height = 1.3 * 3200, timeDirection = "forwards"},
	{type = "box", boxType = "balloon", x = 10 * 3200, y = 14 * 3200, xspeed = 0, yspeed = 0, width = 1.6 * 1600, height = 1.6 * 3200, timeDirection = "forwards"},
	{type = "box", boxType = "balloon", x = 13 * 3200, y = 13 * 3200, xspeed = 0, yspeed = 0, width = 2 * 1600, height = 2 * 3200, timeDirection = "forwards"},
	{type = "box", boxType = "balloon", x = 16 * 3200, y = 12 * 3200, xspeed = 0, yspeed = 0, width = 2.4 * 1600, height = 2.4 * 3200, timeDirection = "forwards"},
	{type = "box", boxType = "balloon", x = 19 * 3200, y = 12 * 3200, xspeed = 0, yspeed = 0, width = 2.9 * 1600, height = 2.9 * 3200, timeDirection = "forwards"},
	{type = "box", boxType = "balloon", x = 22 * 3200, y = 11 * 3200, xspeed = 0, yspeed = 0, width = 3.6 * 1600, height = 3.6 * 3200, timeDirection = "forwards"},
}
triggerSystem =
{
	type = "ConfiguredLua",
	
	luaFiles = {"triggerSystem", "global.basicTriggerSystem"},
	system = [==[ require "triggerSystem" ]==],

	-- C++ still manages offsets and defaults.
	triggerOffsetsAndDefaults = {
		{
			offset = 1,
			default = {0}
		},
		{
			offset = 1,
			default = {0}
		},
		{
			offset = 1,
			default = {1}
		},
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