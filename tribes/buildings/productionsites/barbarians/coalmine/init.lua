dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_coalmine",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Coal Mine",
   icon = dirname .. "menu.png",
   size = "mine",
   enhancement = "barbarians_coalmine_deep",

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
		lore = _[[Ages ago, the Barbarians learned to delve into mountainsides for that black material that feeds their furnaces. <br>
Wood may serve for a household fire and to keep you warm, but when it comes to working with iron or gold, there is no way around coal.]],
		-- #TRANSLATORS: Lore author helptext for a building
		lore_author = "",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Digs coal out of the ground in mountain terrain.",
		-- TRANSLATORS: Note helptext for a building
		note = _"This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any coal.":bformat("1/3"),
		-- TRANSLATORS: Performance helptext for a building
		performance = _"If the food supply is steady, this mine can produce coal in %s on average.":bformat(ngettext("%d second", "%d seconds", 32.5):bformat(32.5))
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
		},
		empty = {
			pictures = path.list_directory(dirname, "empty_\\d+.png"),
			hotspot = { 21, 36 },
		},
	},

   aihints = {
		mines = "coal",
		mines_percent = 30,
		prohibited_till = 1200
   },

	working_positions = {
		barbarians_miner = 1
	},

   inputs = {
		ration = 6
	},
   outputs = {
		"coal"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start mining coal because ...
			descname = _"mining coal",
			actions = {
				"sleep=45000",
				"return=skipped unless economy needs coal",
				"consume=ration",
				"animate=working 20000",
				"mine=coal 2 33 5 17",
				"produce=coal:2"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Main Coal Vein Exhausted",
		message =
			_"This coal mine’s main vein is exhausted. Expect strongly diminished returns on investment." .. " " ..
			-- TRANSLATORS: "it" is a mine.
			_"You should consider enhancing, dismantling or destroying it.",
		delay_attempts = 0
	},
}
