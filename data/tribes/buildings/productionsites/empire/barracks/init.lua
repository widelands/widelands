dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_barracks",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Barracks"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 1,
		granite = 3,
		planks = 2,
		marble = 2,
		marble_column = 2,
		cloth = 1
	},
	return_on_dismantle = {
		granite = 2,
		planks = 1,
		marble = 1
	},

   animations = {
		idle = { -- NOCOM make animations
			pictures = path.list_files(dirname .. "idle_??.png"),
			hotspot = { 52, 64 },
		},
		working = { -- NOCOM make animations
			pictures = path.list_files(dirname .. "idle_??.png"),
			hotspot = { 52, 64 },
		}
	},

	aihints = {
		forced_after = 1000,
		very_weak_ai_limit = 1,
		weak_ai_limit = 3
	},

	working_positions = {
		empire_trainer = 1
	},

	inputs = {
		armor_helmet = 8,
		spear_wooden = 8,
		empire_carrier = 8
	},
	outputs = {
		"empire_soldier",
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start recruiting soldier because ...
			descname = _"recruiting soldier",
			actions = {
				"sleep=15000",
				"return=skipped unless economy needs empire_soldier",
				"consume=armor_helmet spear_wooden empire_carrier",
				"animate=working 15000", -- NOCOM we have no working animation
				"recruit=empire_soldier"
			}
		},
	}
}
