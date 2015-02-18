dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_reed_yard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Reed Yard",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 5,
		granite = 2
	},
	return_on_dismantle = {
		log = 2,
		granite = 1
	},

   helptexts = {
		-- #TRANSLATORS: Lore helptext for a building
		lore = _"‘We grow roofs’",
		-- #TRANSLATORS: Lore author helptext for a building
		lore_author = _"Slogan of the Guild of Gardeners",
		-- #TRANSLATORS: Purpose helptext for a building
		purpose = _"The reed yard cultivates reed that serves two different purposes for the Barbarian tribe.",
		-- #TRANSLATORS: Note helptext for a building
		note = _"Thatch reed is the traditional material for roofing, and it is woven into the extremely durable cloth that they use for their ships’ sails.",
		-- #TRANSLATORS: Performance helptext for a building
		performance = _"A reed yard can produce a sheaf of thatch reed in about %s on average.":bformat(ngettext("%d second", "%d seconds", 65):bformat(65))
   }

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 46, 44 },
		},
	},

   aihints = {
		space_consumer = true,
		forced_after = 0
   },

	working_positions = {
		barbarians_gardener = 1
	},

   outputs = {
		"thatch_reed"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"call=plant_reed",
				"call=harvest_reed",
				"return=skipped"
			}
		},
		plant_reed = {
			-- TRANSLATORS: Completed/Skipped/Did not start planting reed because ...
			descname = _"planting reed",
			actions = {
				"sleep=18000", -- orig sleep=20000 but gardener animation was increased by 2sec
				"worker=plantreed"
			}
		},
		harvest_reed = {
			-- TRANSLATORS: Completed/Skipped/Did not start harvesting reed because ...
			descname = _"harvesting reed",
			actions = {
				"sleep=5000",
				"worker=harvestreed"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Out of Fields",
		message = _"The gardener working at this reed yard has no cleared soil to plant his seeds.",
		delay_attempts = 10
	},
}
