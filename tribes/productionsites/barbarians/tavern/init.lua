dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_tavern",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Tavern",
   size = "medium",
   enhancement = "barbarians_inn"

   buildcost = {
		log = 3,
		blackwood = 2,
		granite = 1,
		thatch_reed = 1
	},
	return_on_dismantle = {
		log = 1,
		blackwood = 1,
		granite = 1
	},

	-- TRANSLATORS: Helptext for a building: Tavern
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 32, 56 },
		},
		working = {
			pictures = { dirname .. "idle_\\d+.png" }, -- TODO(GunChleoc): No animation yet.
			hotspot = { 32, 56 },
		},
	},

   aihints = {
		forced_after = 900
   },

	working_positions = {
		barbarians_innkeeper = 1
	},

   inputs = {
		fish = 4,
		bread_barbarians = 4,
		meat = 4
	},
   outputs = {
		"ration"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
			descname = _"preparing a ration",
			actions = {
				"sleep=14000",
				"return=skipped unless economy needs ration",
				"consume=fish,bread_barbarians,meat",
				"animate=working 19000",
				"produce=ration"
		},
	},
}
