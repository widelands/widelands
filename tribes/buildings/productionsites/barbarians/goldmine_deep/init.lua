dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_goldmine_deep",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Deep Gold Mine",
   icon = dirname .. "menu.png",
   size = "mine",
   buildable = false,
   enhanced_building = true,
   enhancement = "barbarians_goldmine_deeper",

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
		note = _"This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any gold ore.":bformat("2/3"),
		-- TRANSLATORS: Performance helptext for a building
		performance = _"If the food supply is steady, this mine can produce gold ore in %s on average.":bformat(ngettext("%d second", "%d seconds", 19.5):bformat(19.5))
   }

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 21, 37 },
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 21, 37 },
		},
		working = {
			pictures = path.list_directory(dirname, "working_\\d+.png"),
			hotspot = { 21, 37 },
		},
		empty = {
			pictures = path.list_directory(dirname, "empty_\\d+.png"),
			hotspot = { 21, 37 },
		},
	},

   aihints = {
		mines = "gold",
		mines_percent = 60
   },

	working_positions = {
		barbarians_miner = 1,
		barbarians_miner_chief = 1,
	},

   inputs = {
		snack = 6
	},
   outputs = {
		"gold_ore"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start mining gold because ...
			descname = _"mining gold",
			actions = {
				"sleep=42000",
				"return=skipped unless economy needs gold_ore",
				"consume=snack",
				"animate=working 18000",
				"mine=gold 2 66 5 17",
				"produce=gold_ore:2",
				"animate=working 18000",
				"mine=gold 2 66 5 17",
				"produce=gold_ore:2"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Main Gold Vein Exhausted",
		message =
			_"This gold mine’s main vein is exhausted. Expect strongly diminished returns on investment." .. " " ..
			-- TRANSLATORS: "it" is a mine.
			_"You should consider enhancing, dismantling or destroying it.",
		delay_attempts = 0
	},
}
