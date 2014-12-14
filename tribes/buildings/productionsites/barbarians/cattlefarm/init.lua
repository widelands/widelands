dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_cattlefarm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Cattle Farm",
   size = "big",

   buildcost = {
		log = 2,
		granite = 2,
		blackwood = 1
	},
	return_on_dismantle = {
		granite = 1,
		blackwood = 1
	},

	-- #TRANSLATORS: Helptext for a building: Cattle Farm
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 57, 80 },
		},
		working = {
			pictures = { dirname .. "idle_\\d+.png" }, -- TODO(GunChleoc): No animation yet.
			hotspot = { 57, 80 },
		},
	},

   aihints = {
		recruitment = true
   },

	working_positions = {
		barbarians_cattlebreeder = 1
	},

   inputs = {
		wheat = 8,
		water = 8
	},
   outputs = {
		"barbarians_ox"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start rearing cattle because ...
			descname = _"rearing cattle",
			actions = {
				"sleep=15000",
				"return=skipped unless economy needs barbarians_ox",
				"consume=wheat water",
				"playFX=../../../sound/farm/ox 192",
				"animate=working 15000", -- Animation of feeding the cattle
				"recruit=barbarians_ox"
			}
		},
	},
}
