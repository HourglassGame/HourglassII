name = "Tough Choice"
speedOfTime = 24
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
		{1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,1,1,1,1,1},
		{1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1,1},
		{1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1},
		{1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
		{1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	}
}
initialGuy =
{
	arrivalTime = 0,
	arrival =
	{
		x = 4.75 * 3200,
		y = 14 * 3200,
		width = 1600,
		height = 3200,
		facing = "right",
		timeDirection = "forwards",
		pickups = {timeJump = 0, timeReverse = 0, timeGun = 0},
	}
}
initialArrivals =
{
	{type = "box", x = 12.5 * 3200, y = 16 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
	{type = "box", x = 27 * 3200, y = 14 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
	{type = "box", x = 17.5 * 3200, y = 8 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},

}
triggerSystem =
{
	type = "ConfiguredLua",

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
			default = {26 * 3200, 1 * 3200, 0, 0, 0}
		},
		{
			offset = 1,
			default = {1 * 3200, 1 * 3200, 0, 0, 0}
		},
		{
			offset = 1,
			default = {1 * 3200, 1 * 3200, 0, 0, 0}
		},
		{
			offset = 1,
			default = {1 * 3200, 1 * 3200, 0, 0, 0}
		},
		{
			offset = 1,
			default = {7.5 * 3200, 13 * 3200, 0, 0, 0}
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
			default = {0, 0, 0}
		},
		{
			offset = 1,
			default = {11 * 3200, 5 * 3200, 0, 0, 0}
		},
		{
			offset = 1,
			default = {0}
		},
		{
			offset = 1,
			default = {24.5 * 3200, 6 * 3200, 0, 0, 0}
		},
	},
	arrivalLocationsSize = 2,
}