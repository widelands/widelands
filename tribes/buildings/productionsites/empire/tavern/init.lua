dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "empire_tavern",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Tavern",
   icon = dirname .. "menu.png",
   size = "medium",
   enhancement = "empire_inn",

   buildcost = {
		planks = 2,
		granite = 2,
		marble = 1
	},
	return_on_dismantle = {
		planks = 1,
		granite = 1,
		marble = 1
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"Text needed",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Text needed",
		-- TRANSLATORS: Note helptext for a building
		note = "",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"Calculation needed"
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 52, 58 },
		},
		working = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"), -- TODO(GunChleoc): No animation yet.
			hotspot = { 52, 58 },
		},
	},

   aihints = {
		forced_after = 900,
		prohibited_till = 300
   },

	working_positions = {
		empire_innkeeper = 1
	},

   inputs = {
		fish = 5,
		empire_bread = 5,
		meat = 5
	},
   outputs = {
		"ration"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
			descname = _"preparing a ration",
			actions = {
				"sleep=14000",
				"return=skipped unless economy needs ration",
				"consume=empire_bread,fish,meat",
				"animate=working 19000",
				"produce=ration"
			}
		},
	},
}
