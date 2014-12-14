dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "empire_foresters_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Forester’s House",
   size = "small",

   buildcost = {
		log = 1,
		planks = 1,
		granite = 1
	},
	return_on_dismantle = {
		planks = 1,
		granite = 1
	},

	-- #TRANSLATORS: Helptext for a building: Forester’s House
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 52, 54 },
		},
	},

   aihints = {
		space_consumer = true,
		renews_map_resource = "log"
   },

	working_positions = {
		empire_forester = 1
	},

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start planting trees because ...
			descname = _"planting trees",
			actions = {
				"sleep=11000",
				"worker=plant"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Out of Fields",
		message = _"The farmer working at this farm has no cleared soil to plant his seeds.",
		delay_attempts = 10
	},
}
