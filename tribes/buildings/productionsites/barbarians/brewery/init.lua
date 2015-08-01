-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_brewery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Brewery",
   icon = dirname .. "menu.png",
   size = "medium",

   enhancement_cost = {
		log = 3,
		granite = 1,
		thatch_reed = 1
	},
	return_on_dismantle_on_enhanced = {
		log = 1,
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
			hotspot = { 60, 59 },
		},
		working = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"), -- TODO(GunChleoc): No animation yet.
			hotspot = { 60, 59 },
		},
	},

   aihints = {
		prohibited_till = 600,
   },

	working_positions = {
		barbarians_brewer_master = 1,
		barbarians_brewer = 1,
	},

   inputs = {
		water = 8,
		wheat = 8
	},
   outputs = {
		"stout"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start brewing stout because ...
			descname = _"brewing stout",
			actions = {
				"sleep=30000",
				"return=skipped unless economy needs stout",
				"consume=water wheat",
				"animate=working 30000",
				"produce=stout"
			}
		},
	},
}
