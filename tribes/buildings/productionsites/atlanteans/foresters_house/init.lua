dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_foresters_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Forester’s House",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
		log = 1,
		planks = 1,
		granite = 1
	},
	return_on_dismantle = {
		granite = 1,
		log = 1
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"Text needed",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Plants trees in the surrounding area.",
		-- TRANSLATORS: Note helptext for a building
		note = _"The forester’s house needs free space within the working radius to plant the trees.",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"Calculation needed"
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 37, 44 },
		}
	},

   aihints = {
		space_consumer = true,
		renews_map_resource = "log",
		prohibited_till = 50
   },

	working_positions = {
		atlanteans_forester = 1
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
}
