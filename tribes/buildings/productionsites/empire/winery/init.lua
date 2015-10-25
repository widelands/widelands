dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_winery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Winery"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		planks = 1,
		granite = 1,
		marble = 2,
		marble_column = 1
	},
	return_on_dismantle = {
		granite = 1,
		marble = 1,
		marble_column = 1
	},

   animations = {
		idle = {
			template = "idle_??",
			directory = dirname,
			hotspot = { 39, 62 },
		},
		working = {
			template = "idle_??", -- TODO(GunChleoc): No animation yet.
			directory = dirname,
			hotspot = { 39, 62 },
		},
	},

   aihints = {
		forced_after = 600,
		prohibited_till = 600
   },

	working_positions = {
		empire_brewer = 1 -- TODO(GunChleoc): We want a vintner, but don't have the graphics.
	},

   inputs = {
		grape = 8
	},
   outputs = {
		"wine"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start making wine because ...
			descname = _"making wine",
			actions = {
				-- Grapes are only needed for wine, so no need to check if wine is needed
				"sleep=30000",
				"consume=grape:2",
				"animate=working 30000",
				"produce=wine"
			}
		},
	},
}
