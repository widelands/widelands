dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "empire_bakery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Bakery",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 2,
		planks = 2,
		granite = 3
	},
	return_on_dismantle = {
		planks = 1,
		granite = 2
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
			hotspot = { 42, 65 },
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 42, 65 },
			fps = 1
		},
		working = {
			pictures = path.list_directory(dirname, "working_\\d+.png"),
			hotspot = { 43, 65 },
			fps = 2
		},
	},

   aihints = {
		prohibited_till = 600
   },

	working_positions = {
		empire_baker = 1
	},

   inputs = {
		flour = 6,
		water = 6
	},
   outputs = {
		"bread_empire"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start baking bread because ...
			descname = _"baking bread",
			actions = {
				"sleep=15000",
				"return=skipped unless economy needs bread_empire",
				"consume=flour water",
				"animate=working 15000",
				"produce=bread_empire"
			}
		},
	},
}
