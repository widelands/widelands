dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Farm",
   size = "big",

   buildcost = {
		log = 4,
		blackwood = 1,
		granite = 3
	},
	return_on_dismantle = {
		log = 1,
		blackwood = 1,
		granite = 2
	},

	-- #TRANSLATORS: Helptext for a building: Farm
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 72, 73 },
		},
	},

   aihints = {
		space_consumer = true
   },

	working_positions = {
		barbarians_farmer = 1
	},

   outputs = {
		"wheat"
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
				"sleep=4000",
				"worker=harvest"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Out of Fields",
		message = _"The farmer working at this farm has no cleared soil to plant his seeds.",
		delay_attempts = 10
	},
}
