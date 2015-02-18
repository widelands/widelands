dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   name = "barbarians_barrier",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Barrier",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		blackwood = 5,
		grout = 2
	},
	return_on_dismantle = {
		blackwood = 2,
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
			hotspot = { 44, 62 },
			fps = 10
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 44, 62 },
			fps = 1
		},
		unoccupied = {
			pictures = path.list_directory(dirname, "unoccupied_\\d+.png"),
			hotspot = { 44, 62 },
		}
	},

	outputs = {
		"barbarians_soldier",
   },

   aihints = {},

	max_soldiers = 5,
   heal_per_second = 130,
   conquers = 8,
   prefer_heroes = true,

   messages = {
		occupied = _"Your soldiers have occupied your barrier.",
		aggressor = _"Your barrier discovered an aggressor.",
		attack = _"Your barrier is under attack.",
		defeated_enemy = _"The enemy defeated your soldiers at the barrier.",
		defeated_you = _"Your soldiers defeated the enemy at the barrier."
   },
}
