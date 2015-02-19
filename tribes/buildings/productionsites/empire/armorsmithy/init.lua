dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "empire_armorsmithy",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Armor Smithy",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 2,
		granite = 2,
		marble = 2,
		marble_column = 3
	},
	return_on_dismantle = {
		granite = 1,
		marble = 1,
		marble_column = 2
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"Text needed",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Text needed",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"Calculation needed"
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 50, 62 },
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 50, 62 },
			fps = 1
		},
		unoccupied = {
			pictures = path.list_directory(dirname, "unoccupied_\\d+.png"),
			hotspot = { 50, 62 },
		},
		working = {
			pictures = path.list_directory(dirname, "working_\\d+.png"),
			hotspot = { 50, 62 },
			fps = 1
		},
	},

   aihints = {
		prohibited_till = 900
   },

	working_positions = {
		empire_armorsmith = 1
	},

   inputs = {
		iron = 8,
		gold = 8,
		coal = 8,
		cloth = 8
	},
   outputs = {
		"armor_helmet",
		"armor",
		"armor_chain",
		"armor_gilded"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"call=produce_armor_helmet",
				"call=produce_armor",
				"call=produce_armor_chain",
				"call=produce_armor_gilded",
				"return=skipped"
			}
		},
		produce_armor_helmet = {
			-- TRANSLATORS: Completed/Skipped/Did not start forging a helmet because ...
			descname = _"forging a helmet",
			actions = {
				"return=skipped unless economy needs armor_helmet",
				"sleep=32000",
				"consume=iron coal",
				"animate=working 35000",
				"produce=armor_helmet"
			}
		},
		produce_armor = {
			-- TRANSLATORS: Completed/Skipped/Did not start forging a suit of armor because ...
			descname = _"forging a suit of armor",
			actions = {
				"return=skipped unless economy needs armor",
				"sleep=32000",
				"consume=iron coal cloth",
				"animate=working 45000",
				"produce=armor"
			}
		},
		produce_armor_chain = {
			-- TRANSLATORS: Completed/Skipped/Did not start forging a suit of chain armor because ...
			descname = _"forging a suit of chain armor",
			actions = {
				"return=skipped unless economy needs armor_chain",
				"sleep=32000",
				"consume=iron:2 coal cloth",
				"animate=working 45000",
				"produce=armor_chain"
			}
		},
		produce_armor_gilded = {
			-- TRANSLATORS: Completed/Skipped/Did not start forging a suit of gilded armor because ...
			descname = _"forging a suit of gilded armor",
			actions = {
				"return=skipped unless economy needs armor_gilded",
				"sleep=32000",
				"consume=iron:2 coal:2 cloth gold",
				"animate=working 45000",
				"produce=armor_gilded"
			}
		},
	},
}
