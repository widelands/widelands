dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_hunters_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Hunter’s Hut",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
		log = 4,
		granite = 1
	},
	return_on_dismantle = {
		log = 1,
		granite = 1
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _[[‘As silent as a panther,<br> as deft as a weasel,<br> as swift as an arrow,<br> as deadly as a viper.’]],
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"‘The Art of Hunting’",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Hunts animals to produce meat.",
		-- TRANSLATORS: Note helptext for a building
		note = _"The hunter’s hut needs animals to hunt within the work area.",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"The hunter pauses %s before going to work again.":bformat(ngettext("%d second", "%d seconds", 35):bformat(35))
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 44, 44 },
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 44, 44 },
		},
		unoccupied = {
			pictures = path.list_directory(dirname, "unoccupied_\\d+.png"),
			hotspot = { 44, 44 },
		},
	},

   aihints = {
		prohibited_till = 300
   },

	working_positions = {
		barbarians_hunter = 1
	},

   outputs = {
		"meat"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start hunting because ...
			descname = _"hunting",
			actions = {
				"sleep=35000",
				"worker=hunt"
			}
		},
	},
	out_of_resource_notification = {
		-- TRANSLATORS: "Game" means animals that you can hunt
		title = _"Out of Game",
		-- TRANSLATORS: "game" means animals that you can hunt
		message = _"The hunter working out of this hunter’s hut can’t find any game in his work area. Remember that you can build a gamekeeper’s hut to release more game into the wild.",
		productivity_threshold = 33
	},
}
