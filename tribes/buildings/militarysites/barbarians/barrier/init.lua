dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_barrier",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Barrier"),
   helptext_script = dirname .. "helptexts.lua",
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

   animations = {
		idle = {
			template = "idle_??",
			directory = dirname,
			hotspot = { 44, 62 },
			fps = 10
		},
		build = {
			template = "build_??",
			directory = dirname,
			hotspot = { 44, 62 },
		},
		unoccupied = {
			template = "unoccupied_??",
			directory = dirname,
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
