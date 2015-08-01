-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "empire_coalmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Coal Mine",
   icon = dirname .. "menu.png",
   size = "mine",
   enhancement = "empire_coalmine_deep",

   buildcost = {
		log = 4,
		planks = 2
	},
	return_on_dismantle = {
		log = 2,
		planks = 1
	},

   helptexts = {
		-- #TRANSLATORS: Lore helptext for a building
		lore = no_lore_text_yet(),
		-- #TRANSLATORS: Lore author helptext for a building
		lore_author = no_lore_author_text_yet(),
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Digs coal out of the ground in mountain terrain.",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- #TRANSLATORS: Performance helptext for a building
		performance = no_performance_text_yet()
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 49, 49 },
		},
		working = {
			pictures = path.list_directory(dirname, "working_\\d+.png"),
			hotspot = { 49, 49 },
			fps = 10
		},
		empty = {
			pictures = path.list_directory(dirname, "empty_\\d+.png"),
			hotspot = { 49, 49 },
		},
	},

   aihints = {
		mines = "coal",
		mines_percent = 50,
		prohibited_till = 1200
   },

	working_positions = {
		empire_miner = 1
	},

   inputs = {
		ration = 6,
		beer = 6
	},
   outputs = {
		"coal"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start mining coal because ...
			descname = _"mining coal",
			actions = {
				"sleep=45000",
				"return=skipped unless economy needs coal",
				"consume=beer ration",
				"animate=working 20000",
				"mine=coal 2 50 5 17",
				"produce=coal:2",
				"animate=working 20000",
				"mine=coal 2 50 5 17",
				"produce=coal"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Main Coal Vein Exhausted",
		message =
			_"This coal mine’s main vein is exhausted. Expect strongly diminished returns on investment." .. " " ..
			-- TRANSLATORS: "it" is a mine.
			_"You should consider enhancing, dismantling or destroying it.",
	},
}
