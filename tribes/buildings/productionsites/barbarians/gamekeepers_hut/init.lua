dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_gamekeepers_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Gamekeeper’s Hut",
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
		lore = _[[‘He loves the animals and to breed them<br>
			as we love to cook and eat them.’]],
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Barbarian nursery rhyme",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Releases animals into the wild to steady the meat production.",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"The gamekeeper pauses %s before going to work again.":bformat(ngettext("%d second", "%d seconds", 52.5):bformat(52.5))
   }

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 44, 41 },
		},
	},

   aihints = {
		renews_map_resource = "meat",
		prohibited_till = 900
   },

	working_positions = {
		barbarians_gamekeeper = 1
	},

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"sleep=52500",
				"worker=release"
			}
		},
	},
}
