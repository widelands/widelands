dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   name = "barbarians_citadel",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Citadel",
   icon = dirname .. "menu.png",
   size = "big",
   buildable = false,
   enhanced_building = true,

   enhancement_cost = {
		blackwood = 9,
		log = 5,
		granite = 4,
		grout = 2
	},
	return_on_dismantle_on_enhanced = {
		blackwood = 4,
		log = 2,
		granite = 3,
		grout = 1
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"Text needed",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Garrisons soldiers to expand your territory.",
		-- TRANSLATORS: Note helptext for a building
		note = _"If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.",
		-- #TRANSLATORS: Performance helptext for a building
		performance = ""
   }

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 102, 102 },
			fps = 10
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 102, 102 }
		},
		unoccupied = {
			pictures = path.list_directory(dirname, "unoccupied_\\d+.png"),
			hotspot = { 102, 102 }
		}
	},

	outputs = {
		"barbarians_soldier",
   },

	max_soldiers = 12,
   heal_per_second = 220,
   conquers = 12,
   prefer_heroes = true,

   messages = {
		occupied = _"Your soldiers have occupied your citadel.",
		aggressor = _"Your citadel discovered an aggressor.",
		attack = _"Your citadel is under attack.",
		defeated_enemy = _"The enemy defeated your soldiers at the citadel.",
		defeated_you = _"Your soldiers defeated the enemy at the citadel."
   },
}
