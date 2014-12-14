dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   name = "empire_arena",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Arena",
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

	-- #TRANSLATORS: Helptext for a building: Arena
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 81, 82 }
		},
		build = {
			pictures = { dirname .. "build_\\d+.png" },
			hotspot = { 82, 83 }
		}
	},

	working_positions = {
		empire_trainer = 1
	},

	inputs = {
		bread_empire = 10,
		fish = 6,
		meat = 6
	},

	["soldier evade"] = {
		min_level = 0,
		max_level = 0
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
	}

   soldier_capacity = 8,
   trainer_patience = 8
}
