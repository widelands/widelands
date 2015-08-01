-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "empire_hunters_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Hunter’s House",
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
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Hunts animals to produce meat.",
		-- TRANSLATORS: Note helptext for a building
		note = _"The hunter’s house needs animals to hunt within the work area.",
		-- #TRANSLATORS: Performance helptext for a building
		performance = no_performance_text_yet()
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 54, 55 },
		},
	},

	aihints = {},

	working_positions = {
		empire_hunter = 1
	},

   outputs = {
		"meat"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start hunting because ...
			descname = _"hunting",
			actions = {
				"sleep=35000",
				"worker=hunt"
			}
		},
	},
	out_of_resource_notification = {
		-- TRANSLATORS: "Game" means animals that you can hunt
		title = _"Out of Game",
		-- TRANSLATORS: "game" means animals that you can hunt
		message = _"The hunter working out of this hunter’s house can’t find any game in his work area.",
		productivity_threshold = 0
	},
}
