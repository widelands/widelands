dirname = path.dirname(__file__)

tribes:new_warehouse_type {
   name = "atlanteans_port",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Port",
   icon = dirname .. "menu.png",
   size = "port",

   buildcost = {
		log = 3,
		planks = 3,
		granite = 4,
		diamond = 1,
		quartz = 1,
		spidercloth = 3,
		gold = 2
	},
	return_on_dismantle = {
		log = 1,
		planks = 1,
		granite = 2,
		spidercloth = 1,
		gold = 1
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"Text needed",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Text needed",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- #TRANSLATORS: Performance helptext for a building
		performance = ""
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 74, 70 },
			fps = 10
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 74, 70 },
			fps = 1
		}
	},

	aihints = {
		prohibited_till = 900
	},

   conquers = 5,
   heal_per_second = 170,
}
