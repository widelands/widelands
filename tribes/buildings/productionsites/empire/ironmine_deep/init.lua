dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "empire_ironmine_deep",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Deep Iron Mine",
   icon = dirname .. "menu.png",
   size = "mine",
   buildable = false,
   enhanced_building = true,

   enhancement_cost = {
		log = 4,
		planks = 2
	},
	return_on_dismantle_on_enhanced = {
		log = 2,
		planks = 1
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"Text needed",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Digs iron ore out of the ground in mountain terrain.",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"Calculation needed"
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 49, 61 },
		},
		working = {
			pictures = path.list_directory(dirname, "working_\\d+.png"),
			hotspot = { 49, 61 },
			fps = 10
		},
		empty = {
			pictures = path.list_directory(dirname, "empty_\\d+.png"),
			hotspot = { 49, 61 },
		},
	},

   aihints = {
		mines = "gold"
   },

	working_positions = {
		empire_miner = 1,
		empire_miner_master = 1
	},

   inputs = {
		meal = 6,
		beer = 6
	},
   outputs = {
		"iron"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start mining iron because ...
			descname = _"mining iron",
			actions = {
				"sleep=43000",
				"return=skipped unless economy needs iron_ore",
				"consume=meal beer",
				"animate=working 18000",
				"mine=iron 2 100 5 2",
				"produce=iron_ore:2",
				"animate=working 18000",
				"mine=iron 2 100 5 2",
				"produce=iron_ore:2"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Main Iron Vein Exhausted",
		message =
			_"This iron mine’s main vein is exhausted. Expect strongly diminished returns on investment." .. " " ..
			-- TRANSLATORS: "it" is a mine.
			_"This mine can’t be enhanced any further, so you should consider dismantling or destroying it.",
		delay_attempts = 0
	},
}
