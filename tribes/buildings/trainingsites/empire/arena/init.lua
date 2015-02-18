dirname = path.dirname(__file__)

tribes:new_trainingsite_type {
   name = "empire_arena",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Arena",
   icon = dirname .. "menu.png",
   size = "big",
   enhancement = "empire_colosseum",

   buildcost = {
		log = 2,
		granite = 4,
		marble = 5,
		planks = 5,
		marble_column = 2
	},
	return_on_dismantle = {
		log = 1,
		granite = 3,
		marble = 3,
		planks = 2,
		marble_column = 1
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"Text needed",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Trains soldiers in ‘Evade’." .. " " .. _"‘Evade’ increases the soldier’s chance not to be hit by the enemy and so to remain totally unaffected.",
		-- TRANSLATORS: Note helptext for a building
		note = _"Imperial soldiers cannot be trained in ‘Defense’ and will remain at the level with which they came.",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"Calculation needed"
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 81, 82 }
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 82, 83 }
		}
	},

	aihints = {},

	working_positions = {
		empire_trainer = 1
	},

	inputs = {
		bread_empire = 10,
		fish = 6,
		meat = 6
	},
	outputs = {
		"empire_soldier",
   },

	["soldier evade"] = {
		min_level = 0,
		max_level = 0,
		food = {
			{"fish", "meat"},
			{"bread_empire"}
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
				"consume=bread_empire:2 fish,meat",
				"train=soldier evade 0 1"
			}
		},
	},

   soldier_capacity = 8,
   trainer_patience = 8
}
