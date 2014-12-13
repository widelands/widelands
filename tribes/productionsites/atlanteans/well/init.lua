dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_well",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Well",
   size = "small",

   buildcost = {
		log = 2,
		granite = 1,
		planks = 1
	},
	return_on_dismantle = {
		log = 1,
		granite = 1
	},

	-- TRANSLATORS: Helptext for a building: Well
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 31, 32 },
		},
		working = {
			pictures = { dirname .. "idle_\\d+.png" }, -- TODO(GunChleoc): No animation yet.
			hotspot = { 31, 32 },
		},
	},

   aihints = {
		mines_water = true,
		forced_after = 600,
		prohibited_till = 300
   },

	working_positions = {
		atlanteans_carrier = 1
	},

   outputs = {
		"water"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"sleep=20000",
				"animate=working 20000",
				"mine=water 1 100 65 2",
				"produce=water"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Out of Water",
		message = _"The carrier working at this well can’t find any water in his working radius.",
		delay_attempts = 50
	},
}
