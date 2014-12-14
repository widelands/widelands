dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_charcoal_kiln",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Charcoal Kiln",
   size = "medium",

   buildcost = {
		log = 3,
		grout = 2,
		thatch_reed = 2
	},
	return_on_dismantle = {
		log = 2,
		grout = 1
	},

	-- TRANSLATORS: Helptext for a building: Charcoal Kiln
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 47, 54 },
		},
		working = {
			pictures = { dirname .. "idle_\\d+.png" }, -- TODO(GunChleoc): No animation yet.
			hotspot = { 47, 54 },
		},
	},

   aihints = {
		prohibited_till = 900
   },

	working_positions = {
		barbarians_charcoal_burner = 1
	},

   inputs = {
		log = 8
	},
   outputs = {
		"coal"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start producing coal because ...
			descname = _"producing coal",
			actions = {
				"sleep=30000",
				"return=skipped unless economy needs coal",
				"consume=log:6",
				"animate=working 90000", -- Charcoal fires will burn for some days in real life
				"produce=coal"
			}
		},
	},
}
