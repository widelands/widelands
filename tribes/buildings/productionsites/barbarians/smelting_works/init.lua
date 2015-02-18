dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_smelting_works",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Smelting Works",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 3,
		blackwood = 1,
		granite = 1,
		grout = 1,
		thatch_reed = 2
	},
	return_on_dismantle = {
		log = 1,
		blackwood = 1,
		granite = 1
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"Text needed",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Smelts iron ore into iron and gold ore into gold.",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"Calculation needed"
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 44, 58 },
		},
		working = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"), -- TODO(GunChleoc): No animation yet.
			hotspot = { 44, 58 },
		},
	},

   aihints = {
		prohibited_till = 300
   },

	working_positions = {
		barbarians_smelter = 1
	},

   inputs = {
		iron_ore = 8,
		gold_ore = 8,
		coal = 8
	},
   outputs = {
		"iron",
		"gold"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"call=smelt_iron",
				"call=smelt_gold",
				"call=smelt_iron",
				"return=skipped"
			}
		},
		smelt_iron = {
			-- TRANSLATORS: Completed/Skipped/Did not start smelting iron because ...
			descname = _"smelting iron",
			actions = {
				"return=skipped unless economy needs iron",
				"sleep=32000",
				"consume=coal iron_ore",
				"animate=working 35000",
				"produce=iron"
			}
		},
		smelt_gold = {
			-- TRANSLATORS: Completed/Skipped/Did not start smelting gold because ...
			descname = _"smelting gold",
				"return=skipped unless economy needs gold",
				"sleep=32000",
				"consume=coal gold_ore",
				"animate=working 35000",
				"produce=gold"
			}
		},
	},
}
