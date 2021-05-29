name = "UFO"
speedOfTime = 24
speedOfTimeFuture = 0
timelineLength = 10800
environment =
{
	gravity = 10,
	wall =
	{
		width = 32,
		height = 20,
		segmentSize = 3200,
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,1,1,1,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,1,1,1,0,0,1},
		{1,0,0,0,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
		{1,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	}
}
initialGuy =
{
	arrivalTime = 0,
	arrival =
	{
		x = 15.75 * 3200,
		y = 7 * 3200,
		width = 1600,
		height = 3200,
		facing = "right",
		timeDirection = "forwards",
		pickups = {timeJump = 0, timeReverse = 0, timeGun = 0}
	}
}
initialArrivals =
{
	{type = "box", x = 2.5 * 3200, y = 7 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
	{type = "box", x = 28.5 * 3200, y = 7 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
	{type = "box", boxType = "balloon", x = 14.1 * 3200, y = 10 * 3200, xspeed = 0, yspeed = 0, width = 1.3 * 1600, height = 1.3 * 3200, timeDirection = "forwards"},
	{type = "box", boxType = "balloon", x = 17.25 * 3200, y = 10 * 3200, xspeed = 0, yspeed = 0, width = 1.3 * 1600, height = 1.3 * 3200, timeDirection = "forwards"},
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