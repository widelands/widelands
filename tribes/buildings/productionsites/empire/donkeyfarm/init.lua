dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "empire_donkeyfarm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Donkey Farm",
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

	-- #TRANSLATORS: Helptext for a building: Donkey Farm
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 85, 78 },
		},
		working = {
			pictures = { dirname .. "idle_\\d+.png" }, -- TODO(GunChleoc): No animation yet.
			hotspot = { 85, 78 },
		},
	},

   aihints = {
		recruitment = true
   },

	working_positions = {
		empire_donkeybreeder = 1
	},

   inputs = {
		wheat = 8,
		water = 8
	},
   outputs = {
		"empire_donkey"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start rearing donkeys because ...
			descname = _"rearing donkeys",
			actions = {
				"sleep=15000",
				"return=skipped unless economy needs empire_donkey",
				"consume=wheat water",
				"playFX=../../../sound/farm/donkey 192",
				"animate=working 15000", -- Feeding cute little baby donkeys ;)
				"recruit=empire_donkey"
			}
		},
	},
}
