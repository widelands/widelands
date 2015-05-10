dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   name = "barbarians_fortress",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Fortress",
   icon = dirname .. "menu.png",
   size = "big",
   enhancement = "barbarians_citadel",

   buildcost = {
		blackwood = 9,
		log = 5,
		granite = 4,
		grout = 2
	},
	return_on_dismantle = {
		blackwood = 4,
		log = 2,
		granite = 2,
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
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 103, 80 },
			fps = 10
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 103, 80 },
		},
		unoccupied = {
			pictures = path.list_directory(dirname, "unoccupied_\\d+.png"),
			hotspot = { 103, 80 }
		}
	},

   aihints = {
		expansion = true,
		fighting = true,
		mountain_conqueror = true,
		prohibited_till=1500
   },

	outputs = {
		"barbarians_soldier",
   },

	max_soldiers = 8,
   heal_per_second = 170,
   conquers = 11,
   prefer_heroes = true,

   messages = {
		occupied = _"Your soldiers have occupied your fortress.",
		aggressor = _"Your fortress discovered an aggressor.",
		attack = _"Your fortress is under attack.",
		defeated_enemy = _"The enemy defeated your soldiers at the fortress.",
		defeated_you = _"Your soldiers defeated the enemy at the fortress."
   },
}
