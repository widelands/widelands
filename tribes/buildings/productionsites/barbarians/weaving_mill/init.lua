dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_weaving_mill",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Weaving Mill",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
		log = 5,
		granite = 2,
		thatch_reed = 2
	},
	return_on_dismantle = {
		log = 2,
		granite = 2
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"Text needed",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Weaves cloth out of thatch reed.",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"Calculation needed"
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 36, 74 },
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 36, 74 },
			fps = 1
		},
		working = {
			pictures = path.list_directory(dirname, "working_\\d+.png"),
			hotspot = { 36, 74 },
		},
	},

   aihints = {
		prohibited_till = 1200
   },

	working_positions = {
		barbarians_weaver = 1
	},

   inputs = {
		thatch_reed = 8
	},
   outputs = {
		"cloth"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start weaving because ...
			descname = _"weaving",
			actions = {
				"sleep=20000",
				"return=skipped unless economy needs cloth",
				"consume=thatch_reed",
				"animate=working 25000",
				"produce=cloth"
			}
		},
	},
}
