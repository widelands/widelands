dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_spiderfarm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Spider Farm",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
		log = 2,
		granite = 2,
		planks = 2
	},
	return_on_dismantle = {
		granite = 1,
		planks = 1
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"Text needed",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Text needed",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"Calculation needed"
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname,  "idle_\\d+.png"),
			hotspot = { 87, 75 },
		},
		working = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"), -- TODO(GunChleoc): No animation yet.
			hotspot = { 87, 75 },
		}
	},

   aihints = {
		forced_after = 60,
		prohibited_till = 60
   },

	working_positions = {
		atlanteans_spiderbreeder = 1
	},

   inputs = {
		corn = 7,
		water = 7
	},
   outputs = {
		"spider_silk"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"sleep=25000",
				"return=skipped unless economy needs spider_silk",
				"consume=corn water",
				"animate=working 30000",
				"produce=spider_silk"
			}
		},
	},
}
