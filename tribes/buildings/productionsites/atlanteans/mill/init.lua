-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_mill",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Mill",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 3,
		granite = 3,
		planks = 2
	},
	return_on_dismantle = {
		log = 1,
		granite = 2,
		planks = 1
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
			hotspot = { 58, 61 },
		},
		working = {
			pictures = path.list_directory(dirname, "working_\\d+.png"),
			hotspot = { 58, 61 },
			fps = 25
		}
	},

   aihints = {
		prohibited_till = 600
   },

	working_positions = {
		atlanteans_miller = 1
	},

   inputs = {
		corn = 6,
		blackroot = 6
	},
   outputs = {
		"cornmeal",
		"blackroot_flour"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"call=produce_cornmeal",
				"call=produce_blackroot_flour",
				"return=skipped"
			}
		},
		produce_cornmeal = {
			-- TRANSLATORS: Completed/Skipped/Did not start grinding corn because ...
			descname = _"grinding corn",
			actions = {
				"return=skipped when site has blackroot and economy needs blackroot_flour and not economy needs cornmeal",
				"return=skipped unless economy needs cornmeal",
				"sleep=3500",
				"consume=corn",
				"animate=working 15000",
				"produce=cornmeal"
			}
		},
		produce_blackroot_flour = {
			-- TRANSLATORS: Completed/Skipped/Did not start grinding blackrootbecause ...
			descname = _"grinding blackroot",
			actions = {
				-- No check whether we need blackroot_flour because blackroots cannot be used for anything else.
				"return=skipped when site has corn and economy needs cornmeal and not economy needs blackroot_flour",
				"sleep=3500",
				"consume=blackroot",
				"animate=working 15000",
				"produce=blackroot_flour"
			}
		},
	},
}
