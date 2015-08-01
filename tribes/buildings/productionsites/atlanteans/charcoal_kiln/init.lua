-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_charcoal_kiln",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Charcoal Kiln",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 2,
		granite = 3,
		planks = 1
	},
	return_on_dismantle = {
		log = 2,
		granite = 2
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
			hotspot = { 47, 57 },
		},
		working = {
			pictures = path.list_directory(dirname, "working_\\d+.png"),
			hotspot = { 47, 60 },
		},
	},

   aihints = {
		prohibited_till = 1200
   },

	working_positions = {
		atlanteans_charcoal_burner = 1
	},

   inputs = {
		log = 8
	},
   outputs = {
		"coal"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start producing coal because ...
			descname = _"producing coal",
			actions = {
				"sleep=30000",
				"return=skipped unless economy needs coal",
				"consume=log:6",
				"animate=working 90000", -- Charcoal fires will burn for some days in real life
				"produce=coal"
			}
		},

	},
}
