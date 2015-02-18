dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "empire_brewery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Brewery",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 1,
		planks = 2,
		granite = 2
	},
	return_on_dismantle = {
		planks = 1,
		granite = 1
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"Text needed",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Text needed",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"Calculation needed"
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 39, 62 },
		},
		working = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"), -- TODO(GunChleoc): No animation yet.
			hotspot = { 39, 62 },
		},
	},

   aihints = {
		forced_after = 900,
		prohibited_till = 600
   },

	working_positions = {
		empire_brewer = 1
	},

   inputs = {
		water = 7,
		wheat = 7
	},
   outputs = {
		"beer"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start brewing beer because ...
			descname = _"brewing beer",
			actions = {
				"sleep=30000",
				"return=skipped unless economy needs beer",
				"consume=water wheat",
				"animate=working 30000",
				"produce=beer"
			}
		},
	},
}
