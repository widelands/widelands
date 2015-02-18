dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_tavern",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Tavern",
   icon = dirname .. "menu.png",
   size = "medium",
   enhancement = "barbarians_inn"

   buildcost = {
		log = 3,
		blackwood = 2,
		granite = 1,
		thatch_reed = 1
	},
	return_on_dismantle = {
		log = 1,
		blackwood = 1,
		granite = 1
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
   }

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 57, 88 },
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 57, 88 },
		},
		working = {
			pictures = path.list_directory(dirname, "working_\\d+.png"),
			hotspot = { 57, 88 },
		},
	},

   aihints = {
		forced_after = 900
   },

	working_positions = {
		barbarians_innkeeper = 1
	},

   inputs = {
		fish = 4,
		bread_barbarians = 4,
		meat = 4
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
				"consume=fish,bread_barbarians,meat",
				"animate=working 19000",
				"produce=ration"
		},
	},
}
