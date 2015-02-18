dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_granitemine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Granite Mine",
   icon = dirname .. "menu.png",
   size = "mine",

   buildcost = {
		log = 4,
		granite = 2
	},
	return_on_dismantle = {
		log = 2,
		granite = 1
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"‘I can handle tons of granite, man, but no more of your vain prattle.’",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _[[This phrase was the reply Rimbert the miner – later known as Rimbert the loner – gave, when he was asked to remain seated on an emergency meeting at Stonford in the year of the great flood. <br> The same man had all the 244 granite blocks ready only a week later, and they still fortify the city’s levee.]],
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Carves granite out of the rock in mountain terrain.",
		-- TRANSLATORS: Note helptext for a building
		note = _"This mine exploits all of the resource down to the deepest level. But even after having done so, it will still have a %s chance of finding some more granite.":bformat("5%") .. "<br>"
			-- TRANSLATORS: 'It' is a mine
			.. _"It cannot be upgraded."),
		-- TRANSLATORS: Performance helptext for a building
		performance = _"If the food supply is steady, this mine can produce granite in %s on average.":format(ngettext("%d second", "%d seconds", 20):bformat(20))
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 42, 35 },
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 42, 35 },
			fps = 1
		},
		working = {
			pictures = path.list_directory(dirname, "working_\\d+.png"),
			hotspot = { 42, 35 },
		},
		empty = {
			pictures = path.list_directory(dirname, "empty_\\d+.png"),
			hotspot = { 42, 35 },
		},
	},

   aihints = {
		mines = "granite",
		prohibited_till = 900
   },

	working_positions = {
		barbarians_miner = 1
	},

   inputs = {
		ration = 8
	},
   outputs = {
		"granite"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start mining granite because ...
			descname = _"mining granite",
			actions = {
				"sleep=20000",
				"return=skipped unless economy needs granite",
				"consume=ration",
				"animate=working 20000",
				"mine=granite 2 100 5 17",
				"produce=granite:2"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Main Granite Vein Exhausted",
		message =
			_"This granite mine’s main vein is exhausted. Expect strongly diminished returns on investment." .. " " ..
			-- TRANSLATORS: "it" is a mine.
			_" This mine can’t be enhanced any further, so you should consider dismantling or destroying it.",
		delay_attempts = 0
	},
}
