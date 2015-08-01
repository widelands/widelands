-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_gold_spinning_mill",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Gold Spinning Mill",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
		log = 2,
		granite = 1,
		planks = 1
	},
	return_on_dismantle = {
		log = 1,
		granite = 1
	},

   helptexts = {
		-- #TRANSLATORS: Lore helptext for a building
		lore = no_lore_text_yet(),
		-- #TRANSLATORS: Lore author helptext for a building
		lore_author = no_lore_author_text_yet(),
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Spins gold thread out of gold.",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- #TRANSLATORS: Performance helptext for a building
		performance = no_performance_text_yet()
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 33, 64 },
		},
		working = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"), -- TODO(GunChleoc): No animation yet.
			hotspot = { 33, 64 },
		}
	},

   aihints = {
		prohibited_till = 600
   },

	working_positions = {
		atlanteans_carrier = 1
	},

   inputs = {
		gold = 5
	},
   outputs = {
		"gold_thread"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start spinning gold because ...
			descname = _"spinning gold",
			actions = {
				"sleep=15000",
				"return=skipped unless economy needs gold_thread",
				"consume=gold",
				"animate=working 25000",
				"produce=gold_thread"
			}
		},
	},
}
