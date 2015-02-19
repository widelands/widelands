dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_ironmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Iron Mine",
   icon = dirname .. "menu.png",
   size = "mine",
   enhancement = "barbarians_ironmine_deep",

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
		lore = _"‘I look at my own pick wearing away day by day and I realize why my work is important.’",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Quote from an anonymous miner.",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Digs iron ore out of the ground in mountain terrain.",
		-- TRANSLATORS: Note helptext for a building
		note = _"This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any iron ore.":bformat("1/3"),
		-- TRANSLATORS: Performance helptext for a building
		performance = _"If the food supply is steady, this mine can produce iron ore in %s on average.":bformat(ngettext("%d second", "%d seconds", 65):bformat(65))
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 21, 36 },
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 21, 36 },
			fps = 1
		},
		working = {
			pictures = path.list_directory(dirname, "working_\\d+.png"),
			hotspot = { 21, 36 },
			fps = 1
		},
		empty = {
			pictures = path.list_directory(dirname, "empty_\\d+.png"),
			hotspot = { 21, 36 },
		},
	},

   aihints = {
		mines = "iron",
		prohibited_till =900,
		mines_percent = 30
   },

	working_positions = {
		barbarians_miner = 1
	},

   inputs = {
		ration = 6
	},
   outputs = {
		"iron_ore"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start mining iron because ...
			descname = _"mining iron",
			actions = {
				"sleep=45000",
				"return=skipped unless economy needs iron_ore",
				"consume=ration",
				"animate=working 20000",
				"mine=iron 2 33 5 17",
				"produce=iron_ore"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Main Iron Vein Exhausted",
		message =
			_"This iron mine’s main vein is exhausted. Expect strongly diminished returns on investment." .. " " ..
			-- TRANSLATORS: "it" is a mine.
			_"You should consider enhancing, dismantling or destroying it.",
		delay_attempts = 0
	},
}
