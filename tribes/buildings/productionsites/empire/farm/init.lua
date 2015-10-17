dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Farm"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
		planks = 2,
		granite = 2,
		marble = 2,
		marble_column = 2
	},
	return_on_dismantle = {
		planks = 1,
		granite = 1,
		marble = 1,
		marble_column = 1
	},

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 82, 74 },
		},
	},

   aihints = {
		space_consumer = true,
		forced_after = 900
   },

	working_positions = {
		empire_farmer = 1
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
				"playFX=sound/farm farm_animal 192",
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
		message = pgettext("empire_building", "The farmer working at this farm has no cleared soil to plant his seeds."),
		productivity_threshold = 30
	},
}
