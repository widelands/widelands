dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_weaving_mill",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Weaving Mill",
   size = "big",

   buildcost = {
		log = 3,
		granite = 4,
		planks = 2
	},
	return_on_dismantle = {
		log = 1,
		granite = 3,
		planks = 1
	},

	-- TRANSLATORS: Helptext for a building: Weaving Mill
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 65, 69 },
		},
		working = {
			pictures = { dirname .. "idle_\\d+.png" }, -- TODO(GunChleoc): No animation yet.
			hotspot = { 65, 69 },
		}
	},

   aihints = {
		forced_after = 60,
		prohibited_till = 60
   },

	working_positions = {
		atlanteans_weaver = 1
	},

   inputs = {
		spider_silk = 8,
		gold_thread = 4
	},
   outputs = {
		"spidercloth",
		"tabard",
		"tabard_golden"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"call=produce_spidercloth",
				"call=produce_tabard",
				"call=produce_tabard_golden",
				"return=skipped"
			}
		},
		produce_spidercloth = {
			-- TRANSLATORS: Completed/Skipped/Did not start weaving spidercloth because ...
			descname = _"weaving spidercloth",
			actions = {
				"return=skipped unless economy needs spidercloth",
				"sleep=20000",
				"consume=spider_silk",
				"animate=working 20000",
				"produce=spidercloth"
			}
		},
		produce_tabard = {
			-- TRANSLATORS: Completed/Skipped/Did not start tailoring a tabard because ...
			descname = _"tailoring a tabard",
			actions = {
				"return=skipped unless economy needs tabard",
				"sleep=20000",
				"consume=spider_silk",
				"animate=working 20000",
				"produce=tabard"
			}
		},
		produce_tabard_golden = {
			-- TRANSLATORS: Completed/Skipped/Did not start tailoring a golden tabard because ...
			descname = _"tailoring a golden tabard",
			actions = {
				"return=skipped unless economy needs tabard_golden",
				"sleep=20000",
				"consume=spider_silk gold_thread",
				"animate=working 20000",
				"produce=tabard_golden"
			}
		},
	},
}
