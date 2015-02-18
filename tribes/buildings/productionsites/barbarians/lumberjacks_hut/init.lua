dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_lumberjacks_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Lumberjack’s Hut",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
		log = 3
	},
	return_on_dismantle = {
		log = 2
	},

   helptexts = {
		-- #TRANSLATORS: Lore helptext for a building
		lore = _"‘Take 200 hits to fell a tree and you’re a baby. Take 100 and you’re a soldier. Take 50 and you’re a hero. Take 20 and soon you will be a honorable lumberjack.’",
		-- #TRANSLATORS: Lore author helptext for a building
		lore_author = _"Krumta, carpenter of Chat’Karuth",
		-- #TRANSLATORS: Purpose helptext for a building
		purpose = _"Fells trees in the surrounding area and processes them into logs.",
		-- #TRANSLATORS: Note helptext for a building
		note = _"The lumberjack’s hut needs trees to fell within the working radius.",
		-- #TRANSLATORS: Performance helptext for a building
		performance = _"The lumberjack needs %s to fell a tree, not counting the time he needs to reach the destination and go home again.":bformat(ngettext("%d second", "%d seconds", 12):bformat(12))
   }

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 40, 38 },
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 40, 36 },
		},
	},

   aihints = {
		forced_after = 0,
		logproducer = true
   },

	working_positions = {
		barbarians_lumberjack = 1
	},

   outputs = {
		"log"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start felling trees because ...
			descname = _"felling trees",
			actions = {
				"sleep=25000", -- Sleeps shorter than any other tribes.
				"worker=chop"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Out of Trees",
		message = _"The lumberjack working at this lumberjack’s hut can’t find any trees in his working radius. You should consider dismantling or destroying the building or building a ranger’s hut.",
		delay_attempts = 60
	},
}
