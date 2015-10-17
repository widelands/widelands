dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_mill",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Mill"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 3,
		granite = 3,
		marble = 1
	},
	return_on_dismantle = {
		log = 1,
		granite = 2,
		marble = 1
	},

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 41, 87 },
		},
		working = {
			pictures = path.list_directory(dirname, "working_\\d+.png"), -- Let's see the mill turn!
			hotspot = { 41, 87 },
			fps = 25
		},
	},

   aihints = {
		prohibited_till = 600
   },

	working_positions = {
		empire_miller = 1
	},

   inputs = {
		wheat = 6
	},
   outputs = {
		"flour"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start grinding wheat because ...
			descname = _"grinding wheat",
			actions = {
				"sleep=5000",
				"return=skipped unless economy needs flour",
				"consume=wheat",
				"playFX=sound/mill mill_turning 240",
				"animate=working 10000",
				"produce=flour"
			}
		},
	},
}
