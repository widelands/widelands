dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_rangers_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Ranger’s Hut",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
		log = 4
	},
	return_on_dismantle = {
		log = 3
	},

   helptexts = {
		--"Wer zwei Halme dort wachsen lässt, wo sonst nur einer wächst, der ist größer als der größte Feldherr!" – Friedrich der Große
		-- TRANSLATORS: Lore helptext for a building
		lore = _"‘He who can grow two trees where normally only one will grow exceeds the most important general!’",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Chat’Karuth in a conversation with a Ranger",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Plants trees in the surrounding area.",
		-- TRANSLATORS: Note helptext for a building
		note = _"The ranger’s hut needs free space within the working radius to plant the trees.",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"The ranger needs %s to plant a tree, not counting the time he needs to reach the destination and go home again.":bformat(ngettext("%d second", "%d seconds", 5):bformat(5))
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 45, 49 },
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 45, 49 },
		},
		unoccupied = {
			pictures = path.list_directory(dirname, "unoccupied_\\d+.png"),
			hotspot = { 45, 49 },
		},
	},

   aihints = {
		renews_map_resource = "log",
		space_consumer = true
   },

	working_positions = {
		barbarians_ranger = 1
	},

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start planting trees because ...
			descname = _"planting trees",
			actions = {
				"sleep=16000",
				"worker=plant"
			}
		},
	},
}
