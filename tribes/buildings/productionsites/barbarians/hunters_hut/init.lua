dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_hunters_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Hunter’s Hut",
   size = "small",

   buildcost = {
		log = 4,
		granite = 1
	},
	return_on_dismantle = {
		log = 1,
		granite = 1
	},

	-- #TRANSLATORS: Helptext for a building: Hunter’s Hut
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 45, 40 },
		},
	},

   aihints = {
		prohibited_till = 300
   },

	working_positions = {
		barbarians_hunter = 1
	},

   outputs = {
		"meat"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start hunting because ...
			descname = _"hunting",
			actions = {
				"sleep=35000",
				"worker=hunt"
			}
		},
	},
	out_of_resource_notification = {
		-- TRANSLATORS: "Game" means animals that you can hunt
		title = _"Out of Game",
		-- TRANSLATORS: "game" means animals that you can hunt
		message = _"The hunter working out of this hunter’s hut can’t find any game in his working radius. Remember that you can build a gamekeeper’s hut to release more game into the wild.",
		delay_attempts = 10
	},
}
