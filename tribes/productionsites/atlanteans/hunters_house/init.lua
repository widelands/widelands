dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_hunters_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Hunter’s House",
   size = "small",

   buildcost = {
		log = 1,
		planks = 1,
		granite = 1
	},
	return_on_dismantle = {
		granite = 1
	},

	-- TRANSLATORS: Helptext for a building: Hunter’s House
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 36, 44 },
		},
	},

   aihints = {
		prohibited_till = 60
   },

	working_positions = {
		atlanteans_hunter = 1
	},

   outputs = {
		"meat"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start _hunting because ...
			descname = _"_hunting",
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
		message = _"The hunter working out of this hunter’s house can’t find any game in his working radius.",
		delay_attempts = 10
	},
}
