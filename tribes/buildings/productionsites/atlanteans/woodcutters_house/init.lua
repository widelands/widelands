dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_woodcutters_house",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Woodcutter’s House",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
		log = 2,
		planks = 1
	},
	return_on_dismantle = {
		log = 1,
		planks = 1
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"Text needed",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Fells trees in the surrounding area and processes them into logs.",
		-- TRANSLATORS: Note helptext for a building
		note = _"The woodcutter's house needs trees to fell within the working radius.",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"Calculation needed"
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 37, 44 },
		},
	},

   aihints = {
		logproducer = true,
		forced_after = 0
   },

	working_positions = {
		atlanteans_woodcutter = 1
	},

   outputs = {
		"log"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start felling trees because ...
			descname = _"felling trees",
			actions = {
				"sleep=30000", -- Barbarian lumberjack sleeps 25000
				"worker=harvest"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Out of Trees",
		message = _"The woodcutter working at this woodcutter’s house can’t find any trees in his working radius. You should consider dismantling or destroying the building or building a forester’s house.",
		delay_attempts = 60
	},
}
