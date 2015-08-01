-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "empire_weaving_mill",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Weaving Mill",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
		log = 3,
		granite = 4,
		marble = 1
	},
	return_on_dismantle = {
		log = 1,
		granite = 3
	},

   helptexts = {
		-- #TRANSLATORS: Lore helptext for a building
		lore = no_lore_text_yet(),
		-- #TRANSLATORS: Lore author helptext for a building
		lore_author = no_lore_author_text_yet(),
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Weaves cloth out of wool.",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- #TRANSLATORS: Performance helptext for a building
		performance = no_performance_text_yet()
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 65, 62 },
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 65, 62 },
		},
		unoccupied = {
			pictures = path.list_directory(dirname, "unoccupied_\\d+.png"),
			hotspot = { 65, 62 },
		},
		working = {
			pictures = path.list_directory(dirname, "working_\\d+.png"),
			hotspot = { 65, 62 },
			fps = 5
		},
	},

   aihints = {
		prohibited_till = 120
   },

	working_positions = {
		empire_weaver = 1
	},

   inputs = {
		wool = 8
	},
   outputs = {
		"cloth"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start weaving because ...
			descname = _"weaving",
			actions = {
				"sleep=25000",
				"return=skipped unless economy needs cloth",
				"consume=wool",
				"animate=working 15000", -- Unsure of balancing CW
				"produce=cloth"
			}
		},
	},
}
