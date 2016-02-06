dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "atlanteans_building",
   name = "atlanteans_barracks",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "Barracks"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 2,
		granite = 2,
		planks = 2,
		spidercloth = 3,
	},
	return_on_dismantle = {
		log = 1,
		granite = 1,
		planks = 1,
		spidercloth = 1,
	},

   animations = {
		idle = {
			pictures = path.list_files(dirname .. "idle_??.png"),
			hotspot = { 48, 65 },
		},
		working = { -- NOCOM make animations
			pictures = path.list_files(dirname .. "idle_??.png"),
			hotspot = { 48, 65 },
		}
	},

	aihints = {
	},

	working_positions = {
		atlanteans_trainer = 1
	},

	inputs = {
		tabard = 8,
		trident_light = 8
	},
	outputs = {
		"atlanteans_soldier",
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start recruiting soldier because ...
			-- NOCOM this should cost us a carrier as well
			descname = _"recruiting soldier",
			actions = {
				"sleep=15000",
				"return=skipped unless economy needs atlanteans_soldier",
				"consume=tabard trident_light",
				"animate=working 15000", -- NOCOM we have no working animation
				"recruit=atlanteans_soldier"
			}
		},
	}
}
