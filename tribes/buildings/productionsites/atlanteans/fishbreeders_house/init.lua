-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_fishbreeders_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Fish Breeder’s House",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
		log = 1,
		planks = 1,
		granite = 1
	},
	return_on_dismantle = {
		granite = 1
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
			hotspot = { 34, 42 },
		},
	},

   aihints = {
		needs_water = true,
		renews_map_resource = "fish",
		prohibited_till = 300
   },

	working_positions = {
		atlanteans_fishbreeder = 1
	},

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start breeding fish because ...
			descname = _"breeding fish",
			actions = {
				"sleep=24000",
				"worker=breed"
			}
		},
	},
}
