name = "Sealed Room"
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
		{1,1,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1},
		{1,1,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1},
		{1,1,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1},
		{1,1,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1},
		{1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1},
		{1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1},
		{1,1,1,1,1,1,1,1,0,0,1,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,1,1},
		{1,1,1,1,1,1,1,1,0,0,1,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,1,1},
		{1,1,1,1,1,1,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1},
		{1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1},
		{1,1,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1},
		{1,1,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1},
		{1,1,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1},
		{1,1,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	}
}
initialGuy = 
{
	arrivalTime = 10800 / 2,
	arrival = 
	{
		x = 3.25 * 3200,
		y = 2.5 * 3200,
		width = 1600,
		height = 3200,
		facing = "right",
		timeDirection = "forwards",
		pickups = {timeJump = 0, timeReverse = 0, timeGun = 0}
	}
}
initialArrivals =
{
	{type = "box", boxType = "light", x = 12.5 * 3200, y = 9 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
	{type = "box", boxType = "light", x = 18.5 * 3200, y = 9 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
	{type = "box", boxType = "crate", x = 24.5 * 3200, y = 9 * 3200, xspeed = 0, yspeed = 0, size = 3200, timeDirection = "forwards"},
}
triggerSystem = 
{
	type = "ConfiguredLua",
	
	luaFiles = {"triggerSystem", "global.basicTriggerSystem"},
	--This describes a lua chunk which will be loaded by TriggerFrameState
	--to do its stuff.
	system = [==[require "triggerSystem"]==],
	triggerOffsetsAndDefaults = {
		-- Left column
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
			default = {12 * 3200, 6 * 3200, 0, 0, 0}
		},
		{
			offset = 1,
			default = {17 * 3200, 9 * 3200, 0, 0, 0}
		},
		-- Middle column
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
			default = {18 * 3200, 6 * 3200, 0, 0, 0}
		},
		{
			offset = 1,
			default = {23.25 * 3200, 9 * 3200, 0, 0, 0}
		},
		-- Right column
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
			default = {24 * 3200, 6 * 3200, 0, 0, 0}
		},
		{
			offset = 1,
			default = {29.5 * 3200, 9 * 3200, 0, 0, 0}
		},
		-- Other triggers
		{
			offset = 1,
			default = {0}
		},
		{
			offset = 1,
			default = {12 * 3200, 10 * 3200, 0, 0, 0}
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
		}
	},
	arrivalLocationsSize = 7
}

