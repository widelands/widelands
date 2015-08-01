-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_trainingsite_type {
   name = "empire_colosseum",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Colosseum",
   icon = dirname .. "menu.png",
   size = "big",

   enhancement_cost = {
		planks = 2,
		granite = 4,
		marble = 4,
		cloth = 2,
		gold = 4,
		marble_column = 4
	},
	return_on_dismantle_on_enhanced = {
		planks = 1,
		granite = 2,
		marble = 2,
		gold = 2,
		marble_column = 2
	},

   helptexts = {
		-- #TRANSLATORS: Lore helptext for a building
		lore = no_lore_text_yet(),
		-- #TRANSLATORS: Lore author helptext for a building
		lore_author = no_lore_author_text_yet(),
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Trains soldiers in ‘Evade’." .. " " .. _"‘Evade’ increases the soldier’s chance not to be hit by the enemy and so to remain totally unaffected.",
		-- TRANSLATORS: Note helptext for a building
		note = _"Imperial soldiers cannot be trained in ‘Defense’ and will remain at the level with which they came.",
		-- #TRANSLATORS: Performance helptext for a building
		performance = no_performance_text_yet()
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 81, 106 }
		}
	},

	aihints = {
		trainingsite_type = "basic"
	},

	working_positions = {
		empire_trainer = 1
	},

	inputs = {
		empire_bread = 10,
		fish = 6,
		meat = 6
	},
	outputs = {
		"empire_soldier",
   },

	["soldier evade"] = {
		min_level = 0,
		max_level = 1,
		food = {
			{"fish", "meat"},
			{"empire_bread"}
		}
	},

	programs = {
		sleep = {
			-- TRANSLATORS: Completed/Skipped/Did not start sleeping because ...
			descname = _"sleeping",
			actions = {
				"sleep=5000",
				"check_soldier=soldier attack 9", -- dummy check to get sleep rated as skipped - else it will change statistics
			}
		},
		upgrade_soldier_evade_0 = {
			-- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
			descname = _"upgrading soldier evade from level 0 to level 1",
			actions = {
				"check_soldier=soldier evade 0", -- Fails when aren't any soldier of level 0 evade
				"sleep=30000",
				"check_soldier=soldier evade 0", -- Because the soldier can be expelled by the player
				"consume=empire_bread:2 fish,meat",
				"train=soldier evade 0 1"
			}
		},
		upgrade_soldier_evade_1 = {
			-- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
			descname = _"upgrading soldier evade from level 1 to level 2",
			actions = {
				"check_soldier=soldier evade 1", -- Fails when aren't any soldier of level 1 evade
				"sleep=30000",
				"check_soldier=soldier evade 1", -- Because the soldier can be expelled by the player
				"consume=empire_bread:2 fish,meat:2",
				"train=soldier evade 1 2"
			}
		},
	},

   soldier_capacity = 8,
   trainer_patience = 9
}
