dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_horsefarm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Horse Farm",
   size = "big",

   buildcost = {
		log = 2,
		granite = 2,
		planks = 1
	},
	return_on_dismantle = {
		log = 1,
		granite = 2
	},

	-- TRANSLATORS: Helptext for a building: Horse Farm
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 81, 62 },
		},
		working = {
			pictures = { dirname .. "idle_\\d+.png" }, -- TODO(GunChleoc): No animation yet.
			hotspot = { 81, 62 },
		}
	},

   aihints = {
		recruitment = true
   },

	working_positions = {
		atlanteans_horsebreeder = 1
	},

   inputs = {
		corn = 8,
		water = 8
	},
   outputs = {
		"atlanteans_horse"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start breeding horses because ...
			descname = _"breeding horses",
			actions = {
				"sleep=15000",
				"return=skipped unless economy needs atlanteans_horse",
				"consume=corn water",
				"playFX=../../../sound/farm/horse 192",
				"animate=working 15000", -- Feeding cute little foals ;)
				"recruit=atlanteans_horse"
			}
		},

	},
}
