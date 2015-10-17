dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_tower",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Tower"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",
   vision_range = 17,

   buildcost = {
		blackwood = 7,
		log = 1,
		granite = 4
	},
	return_on_dismantle = {
		blackwood = 3,
		granite = 3
	},

   animations = {
		idle = {
			template = "idle_??",
			directory = dirname,
			hotspot = { 48, 84 },
			fps = 10
		},
		build = {
			template = "build_??",
			directory = dirname,
			hotspot = { 48, 84 },
		},
		unoccupied = {
			template = "unoccupied_??",
			directory = dirname,
			hotspot = { 48, 84 }
		}
	},

   aihints = {
		expansion = true,
		fighting = true,
		mountain_conqueror = true,
		prohibited_till = 1200
   },

	outputs = {
		"barbarians_soldier",
   },

	max_soldiers = 5,
   heal_per_second = 150,
   conquers = 8,
   prefer_heroes = true,

   messages = {
		occupied = _"Your soldiers have occupied your tower.",
		aggressor = _"Your tower discovered an aggressor.",
		attack = _"Your tower is under attack.",
		defeated_enemy = _"The enemy defeated your soldiers at the tower.",
		defeated_you = _"Your soldiers defeated the enemy at the tower."
   },
}
