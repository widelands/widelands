dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_well",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Well",
   size = "small",

   buildcost = {
		log = 4
	},
	return_on_dismantle = {
		log = 2
	},

	-- TRANSLATORS: Helptext for a building: Well
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 19, 33 },
		},
		build = {
			pictures = { dirname .. "build_\\d+.png" },
			hotspot = { 19, 33 },
		},
		working = {
			pictures = { dirname .. "working_\\d+.png" },
			hotspot = { 19, 33 },
		},
	},

   aihints = {
		mines_water = true,
		prohibited_till = 600
   },

	working_positions = {
		barbarians_carrier = 1
	},

   outputs = {
		"water"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"call=plant_wheat",
				"call=harvest_wheat",
				"playFX=../../../sound/farm/farm_animal 192",
				"return=skipped"
			}
		},
		plant_wheat = {
			-- TRANSLATORS: Completed/Skipped/Did not start planting wheat because ...
			descname = _"planting wheat",
			actions = {
				"sleep=14000",
				"worker=plant"
			}
		},
		harvest_wheat = {
			-- TRANSLATORS: Completed/Skipped/Did not start harvesting wheat because ...
			descname = _"harvesting wheat",
			actions = {
				"sleep=20000",
				"animate=working 20000",
				"mine=water 1 100 65 2",
				"produce=water"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Out of Water",
		message = _"The carrier working at this well can’t find any water in his working radius.",
		delay_attempts = 50
	},
}
