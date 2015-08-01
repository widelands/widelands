-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_scouts_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Scout’s House",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
		log = 2,
		granite = 1
	},
	return_on_dismantle = {
		log = 1
	},

   helptexts = {
		-- #TRANSLATORS: Lore helptext for a building
		lore = no_lore_text_yet(),
		-- #TRANSLATORS: Lore author helptext for a building
		lore_author = no_lore_author_text_yet(),
		-- #TRANSLATORS: Purpose helptext for a building
		purpose = no_purpose_text_yet(),
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- #TRANSLATORS: Performance helptext for a building
		performance = no_performance_text_yet()
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 41, 44 },
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 41, 44 },
		}
	},

	aihints = {},

	working_positions = {
		atlanteans_scout = 1
	},

   inputs = {
		smoked_fish = 2,
		atlanteans_bread = 2
	},

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start scouting because ...
			descname = _"scouting",
			actions = {
				"sleep=30000",
				"consume=smoked_fish",
				"worker=scout",
				"sleep=30000",
				"consume=atlanteans_bread",
				"worker=scout"
			}
		},
	},
}
