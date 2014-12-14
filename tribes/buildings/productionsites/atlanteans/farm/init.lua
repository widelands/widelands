dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_farm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Farm",
   size = "big",

   buildcost = {
		log = 3,
		granite = 2,
		planks = 2,
		spidercloth = 1
	},
	return_on_dismantle = {
		log = 1,
		granite = 1,
		planks = 1
	},

	-- #TRANSLATORS: Helptext for a building: Farm
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 74, 60 },
		},
	},

   aihints = {
		space_consumer = true,
		 -- Farm needs spidercloth to be built and spidercloth needs corn for production
		 -- -> farm should be built ASAP!
		forced_after = 180,
		prohibited_till = 120
   },

	working_positions = {
		atlanteans_farmer = 1
	},

   outputs = {
		"corn"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"call=plant_corn",
				"call=harvest_corn",
				"playFX=../../../sound/farm/farm_animal 192",
				"return=skipped"
			}
		},
		plant_corn = {
			-- TRANSLATORS: Completed/Skipped/Did not start planting corn because ...
			descname = _"planting corn",
			actions = {
				"sleep=14000",
				"worker=plant"
			}
		},
		harvest_corn = {
			-- TRANSLATORS: Completed/Skipped/Did not start harvesting corn because ...
			descname = _"harvesting corn",
			actions = {
				"sleep=4000",
				"worker=harvest"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Out of Fields",
		message = _"The farmer working at this farm has no cleared soil to plant his seeds.",
		delay_attempts = 10
	},
}
