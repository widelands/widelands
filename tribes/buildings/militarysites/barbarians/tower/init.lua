dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   name = "barbarians_tower",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Tower",
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

	-- TRANSLATORS: Helptext for a militarysite: Tower
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 48, 84 },
			fps = 10
		},
		build = {
			pictures = { dirname .. "build_\\d+.png" },
			hotspot = { 48, 84 }
		},
		unoccupied = {
			pictures = { dirname .. "unoccupied_\\d+.png" },
			hotspot = { 48, 84 }
		}
	},

	max_soldiers = 5,
   heal_per_second = 150,
   conquers = 8,
   prefer_heroes = true,

   aihints = {
		expansion = true,
		fighting = true,
		mountain_conqueror = true,
		prohibited_till = 1500
   },

   messages = {
		occupied = _"Your soldiers have occupied your tower.",
		aggressor = _"Your tower discovered an aggressor.",
		attack = _"Your tower is under attack.",
		defeated_enemy = _"The enemy defeated your soldiers at the tower.",
		defeated_you = _"Your soldiers defeated the enemy at the tower."
   },
}
