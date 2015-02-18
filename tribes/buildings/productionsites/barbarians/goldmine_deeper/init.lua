dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_goldmine_deeper",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Deeper Gold Mine",
   icon = dirname .. "menu.png",
   size = "mine",
   buildable = false,
   enhanced_building = true,

   enhancement_cost = {
		log = 4,
		granite = 2
	},
	return_on_dismantle_on_enhanced = {
		log = 2,
		granite = 1
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _[[‘Soft and supple.<br> And yet untouched by time and weather.<br> Rays of sun, wrought into eternity ...’]],
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _[[Excerpt from ‘Our Treasures Underground’,<br> a traditional Barbarian song.]],
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Digs gold ore out of the ground in mountain terrain.",
		-- TRANSLATORS: Note helptext for a building
		note = _"This mine exploits all of the resource down to the deepest level. But even after having done so, it will still have a %s chance of finding some more gold ore.":bformat("10%"),
		-- TRANSLATORS: Performance helptext for a building
		performance = _"If the food supply is steady, this mine can produce gold ore in %s on average.":bformat(ngettext("%d second", "%d seconds", 18.5):bformat(18.5))
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 60, 37 },
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 60, 37 },
			fps = 1
		},
		working = {
			pictures = path.list_directory(dirname, "working_\\d+.png"),
			hotspot = { 60, 37 },
		},
		empty = {
			pictures = path.list_directory(dirname, "empty_\\d+.png"),
			hotspot = { 60, 37 },
		},
	},

   aihints = {
		mines = "gold",
   },

	working_positions = {
		barbarians_miner = 1,
		barbarians_miner_chief = 1,
		barbarians_miner_master = 1,
	},

   inputs = {
		meal = 6
	},
   outputs = {
		"gold_ore"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start mining gold because ...
			descname = _"mining gold",
			actions = {
				"sleep=40000",
				"return=skipped unless economy needs gold_ore",
				"consume=meal",
				"animate=working 16000",
				"mine=gold 2 100 10 2",
				"produce=gold_ore:2",
				"animate=working 18000",
				"mine=gold 2 100 10 2",
				"produce=gold_ore:2"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Main Gold Vein Exhausted",
		message =
			_"This gold mine’s main vein is exhausted. Expect strongly diminished returns on investment." .. " " ..
			-- TRANSLATORS: "it" is a mine.
			_"This mine can’t be enhanced any further, so you should consider dismantling or destroying it.",
		delay_attempts = 0
	},
}
