dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_bakery",
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
			hotspot = { 52, 63 },
		},
		working = {
			pictures = path.list_directory(dirname, "working_\\d+.png"),
			hotspot = { 52, 75 },
			fps = 20
		}
	},

   aihints = {
		forced_after = 1200,
		prohibited_till = 900
   },

	working_positions = {
		atlanteans_baker = 1
	},

   inputs = {
		blackroot_flour = 4,
		cornmeal = 4,
		water = 8
	},
   outputs = {
		"bread_atlanteans"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start baking bread because ...
			descname = _"baking bread",
			actions = {
				"sleep=35000",
				"return=skipped unless economy needs bread_atlanteans",
				"consume=water:2 blackroot_flour cornmeal",
				"animate=working 30000",
				"produce=bread_atlanteans:2"
			}
		},
	},
}
