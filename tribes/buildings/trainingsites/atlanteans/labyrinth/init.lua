dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   name = "atlanteans_labyrinth",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Labyrinth",
   size = "big",

   buildcost = {
		log = 3,
		granite = 4,
		planks = 5,
		spidercloth = 5,
		diamond = 2,
		quartz = 2
	},
	return_on_dismantle = {
		log = 1,
		granite = 3,
		planks = 2,
		spidercloth = 2,
		diamond = 1,
		quartz = 1
	},

	-- TRANSLATORS: Helptext for a building: Labyrinth
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 80, 88 },
		}
	},

	aihints = {
		prohibited_till = 2700
	}

	working_positions = {
		atlanteans_trainer = 1
	},

	inputs = {
		bread_atlanteans = 10,
		smoked_fish = 6,
		smoked_meat = 6,
		shield_steel = 4,
		shield_advanced = 4,
		tabard_golden = 5
	},

	["soldier defense"] = {
		min_level = 0,
		max_level = 1
	},
	["soldier hp"] = {
		min_level = 0,
		max_level = 0
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
				"check_soldier=soldier attack 9" -- dummy check to get sleep rated as skipped - else it will change statistics
			}
		},
		upgrade_soldier_defense_0 = {
			-- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
			descname = _"upgrading soldier defense from level 0 to level 1",
			actions = {
				"check_soldier=soldier defense 0", -- Fails when aren't any soldier of level 0 defense
				"sleep=30000",
				"check_soldier=soldier defense 0", -- Because the soldier can be expulsed by the player
				"consume=bread_atlanteans smoked_fish,smoked_meat shield_steel",
				"train=soldier defense 0 1"
			}
		},
		upgrade_soldier_defense_1 = {
			-- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
			descname = _"upgrading soldier defense from level 1 to level 2",
			actions = {
				"check_soldier=soldier defense 1", -- Fails when aren't any soldier of level 1 defense
				"sleep=30000",
				"check_soldier=soldier defense 1", -- Because the soldier can be expelled by the player
				"consume=bread_atlanteans smoked_fish,smoked_meat shield_advanced",
				"train=soldier defense 1 2"
			}
		},
		upgrade_soldier_hp_0 = {
			-- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
			descname = _"upgrading soldier health from level 0 to level 1",
			actions = {
				"check_soldier=soldier hp 0", -- Fails when aren't any soldier of level 0 hp
				"sleep=30000",
				"check_soldier=soldier hp 0", -- Because the soldier can be expelled by the player
				"consume=smoked_fish,smoked_meat:2 tabard_golden",
				"train=soldier hp 0 1"
			}
		},
		upgrade_soldier_evade_0 = {
			-- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
			descname = _"upgrading soldier evade from level 0 to level 1",
			actions = {
				"check_soldier=soldier evade 0", -- Fails when aren't any soldier of level 0 evade
				"sleep=30000",
				"check_soldier=soldier evade 0", -- Because the soldier can be expelled by the player
				"consume=bread_atlanteans smoked_fish,smoked_meat:2",
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
				"consume=bread_atlanteans:2 smoked_fish,smoked_meat:2",
				"train=soldier evade 1 2"
			}
		},
	}

   soldier_capacity = 8,
   trainer_patience = 20
}
