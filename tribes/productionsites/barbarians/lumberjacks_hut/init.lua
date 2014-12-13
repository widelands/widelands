dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_lumberjacks_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Lumberjack’s Hut",
   size = "small",

   buildcost = {
		log = 3
	},
	return_on_dismantle = {
		log = 2
	},

	-- TRANSLATORS: Helptext for a building: Lumberjack’s Hut
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 40, 38 },
		},
		build = {
			pictures = { dirname .. "build_\\d+.png" },
			hotspot = { 40, 36 },
		},
	},

   aihints = {
		forced_after = 0,
		logproducer = true
   },

	working_positions = {
		barbarians_lumberjack = 1
	},

   outputs = {
		"log"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start felling trees because ...
			descname = _"felling trees",
			actions = {
				"sleep=25000", -- Sleeps shorter than any other tribes.
				"worker=chop"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Out of Trees",
		message = _"The lumberjack working at this lumberjack’s hut can’t find any trees in his working radius. You should consider dismantling or destroying the building or building a rangers’s hut.",
		delay_attempts = 60
	},
}
