-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_smokery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Smokery",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 1,
		granite = 4,
		planks = 1,
		spidercloth = 1
	},
	return_on_dismantle = {
		granite = 3
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
			hotspot = { 53, 58 },
		},
		working = {
			pictures = path.list_directory(dirname, "working_\\d+.png"),
			hotspot = { 53, 68 },
			fps = 20
		}
	},

   aihints = {
		forced_after = 900,
		prohibited_till = 180
   },

	working_positions = {
		atlanteans_smoker = 1
	},

   inputs = {
		fish = 4,
		meat = 4,
		log = 8
	},
   outputs = {
		"smoked_meat",
		"smoked_fish"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"call=smoke_fish",
				"call=smoke_meat",
				"call=smoke_fish",
				"return=skipped"
			}
		},
		smoke_meat = {
			-- TRANSLATORS: Completed/Skipped/Did not start smoking meat because ...
			descname = _"smoking meat",
			actions = {
				"return=skipped when site has fish and economy needs smoked_fish and not economy needs smoked_meat",
				"return=skipped unless economy needs smoked_meat",
				"sleep=30000",
				"consume=meat:2 log",
				"animate=working 30000",
				"produce=smoked_meat:2"
			}
		},
		smoke_fish = {
			-- TRANSLATORS: Completed/Skipped/Did not start smoking fish because ...
			descname = _"smoking fish",
			actions = {
				"return=skipped when site has meat and economy needs smoked_meat and not economy needs smoked_fish",
				"return=skipped unless economy needs smoked_fish",
				"sleep=30000",
				"consume=fish:2 log",
				"animate=working 30000",
				"produce=smoked_fish:2"
			}
		},
	},
}
