dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_fishbreeders_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Fish Breeder’s House",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
		log = 1,
		planks = 1,
		granite = 1
	},
	return_on_dismantle = {
		granite = 1
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
   }

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 34, 42 },
		},
	},

   aihints = {
		needs_water = true,
		renews_map_resource = "fish",
		prohibited_till = 900
   },

	working_positions = {
		atlanteans_fishbreeder = 1
	},

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start breeding fish because ...
			descname = _"breeding fish",
			actions = {
				"sleep=24000",
				"worker=breed"
			}
		},
	},
}
