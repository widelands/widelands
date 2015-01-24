dirname = path.dirname(__file__)

tribes:new_trainingsite_type {
   name = "barbarians_battlearena",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Battle Arena",
   size = "big",

   buildcost = {
		log = 6,
		granite = 4,
		grout = 6,
		gold = 4,
		thatch_reed = 3
	},
	return_on_dismantle = {
		log = 3,
		granite = 3,
		grout = 3,
		gold = 2,
		thatch_reed = 1
	},

	-- #TRANSLATORS: Helptext for a building: Battle Arena
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 110, 72 },
			fps = 10
		},
		build = {
			pictures = { dirname .. "build_\\d+.png" },
			hotspot = { 110, 72 }
		},
		unoccupied = {
			pictures = { dirname .. "unoccupied_\\d+.png" },
			hotspot = { 110, 72 }
		},
		working = {
			pictures = { dirname .. "working_\\d+.png" },
			hotspot = { 110, 72 },
			fps = 10
		}
	},

	aihints = {
		prohibited_till = 2700
	}

	working_positions = {
		barbarians_trainer = 1
	},

	inputs = {
		bread_barbarians = 10,
		fish = 6,
		meat = 6,
		stout = 6
	},

	["soldier evade"] = {
		min_level = 0,
		max_level = 1
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
				"sleep=15000",
				"check_soldier=soldier evade 0", -- Because the soldier can be expelled by the player
				"consume=bread_barbarians fish,meat stout",
				"animate=working 15000",
				"train=soldier evade 0 1"
			}
		},
		upgrade_soldier_evade_1 = {
			-- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
			descname = _"upgrading soldier evade from level 1 to level 2",
			actions = {
				"check_soldier=soldier evade 1", -- Fails when aren't any soldier of level 1 evade
				"sleep=15000",
				"check_soldier=soldier evade 1", -- Because the soldier can be expulsed by the player
				"consume=bread_barbarians:2 fish,meat stout",
				"animate=working 15000",
				"train=soldier evade 1 2"
			}
		},
	}

   soldier_capacity = 8,
   trainer_patience = 3
}
