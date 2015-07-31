dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_coalmine_deeper",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Deeper Coal Mine",
   icon = dirname .. "menu.png",
   size = "mine",

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
		lore = _[[Ages ago, the Barbarians learned to delve into mountainsides for that black material that feeds their furnaces. <br>
Wood may serve for a household fire and to keep you warm, but when it comes to working with iron or gold, there is no way around coal.]],
		-- #TRANSLATORS: Lore author helptext for a building
		lore_author = "",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Digs coal out of the ground in mountain terrain.",
		-- TRANSLATORS: Note helptext for a building
		note = _"This mine exploits all of the resource down to the deepest level. But even after having done so, it will still have a %s chance of finding some more coal.":bformat("10%"),
		-- TRANSLATORS: Performance helptext for a building
		performance = _"If the food supply is steady, this mine can produce coal in %s on average.":bformat(ngettext("%d second", "%d seconds", 14.4):bformat(14.4))
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 60, 37 },
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 60, 37 },
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
		mines = "coal"
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
		"coal"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start mining coal because ...
			descname = _"mining coal",
			actions = {
				"sleep=40000",
				"return=skipped unless economy needs coal",
				"consume=meal",
				"animate=working 16000",
				"mine=coal 2 100 10 2",
				"produce=coal:2",
				"animate=working 16000",
				"mine=coal 2 100 10 2",
				"produce=coal:3"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Main Coal Vein Exhausted",
		message =
			_"This coal mine’s main vein is exhausted. Expect strongly diminished returns on investment." .. " " ..
			-- TRANSLATORS: "it" is a mine.
			_"This mine can’t be enhanced any further, so you should consider dismantling or destroying it.",
	},
}
