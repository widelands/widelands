dirname = path.dirname(__file__)

tribes:new_trainingsite_type {
   name = "barbarians_battlearena",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Battle Arena",
   icon = dirname .. "menu.png",
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

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"‘No better friend you have in battle than the enemy’s blow that misses.’",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Said to originate from Neidhardt, the famous trainer.",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Trains soldiers in ‘Evade’." .. " " .. _"‘Evade’ increases the soldier’s chance not to be hit by the enemy and so to remain totally unaffected.",
		-- TRANSLATORS: Note helptext for a building
		note = _"Barbarian soldiers cannot be trained in ‘Defense’ and will remain at their initial level.",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"If all needed wares are delivered in time, a battle arena can train evade for one soldier from 0 to the highest level in %1$s and %2$s on average.":bformat(ngettext("%d minute", "%d minutes", 1):bformat(1), ngettext("%d second", "%d seconds", 10):bformat(10))
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 110, 72 },
			fps = 10
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 110, 72 },
			fps = 1
		},
		unoccupied = {
			pictures = path.list_directory(dirname, "unoccupied_\\d+.png"),
			hotspot = { 110, 72 }
		},
		working = {
			pictures = path.list_directory(dirname, "working_\\d+.png"),
			hotspot = { 110, 72 },
			fps = 10
		}
	},

	aihints = {
		prohibited_till = 2700
	},

	working_positions = {
		barbarians_trainer = 1
	},

	inputs = {
		bread_barbarians = 10,
		fish = 6,
		meat = 6,
		stout = 6
	},
	outputs = {
		"barbarians_soldier",
   },

	["soldier evade"] = {
		min_level = 0,
		max_level = 1,
		food = {
			{"fish", "meat"},
			{"stout"},
			{"bread_barbarians"}
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
	},

   soldier_capacity = 8,
   trainer_patience = 3
}
