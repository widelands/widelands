-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "empire_piggery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Piggery",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
		log = 2,
		granite = 2,
		marble_column = 2
	},
	return_on_dismantle = {
		log = 1,
		granite = 1,
		marble = 1
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
			hotspot = { 82, 74 },
		},
		working = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"), -- TODO(GunChleoc): No animation yet.
			hotspot = { 82, 74 },
		},
	},

	aihints = {
		forced_after = 9000
	},

	working_positions = {
		empire_pigbreeder = 1
	},

   inputs = {
		wheat = 7,
		water = 7
	},
   outputs = {
		"meat"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start raising pigs because ...
			descname = _"raising pigs",
			actions = {
				"sleep=25000",
				"return=skipped unless economy needs meat",
				"consume=water wheat",
				"playFX=sound/farm/farm_animal 180",
				"animate=working 30000",
				"produce=meat"
			}
		},
	},
}
