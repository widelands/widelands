dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   name = "barbarians_fortress",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Fortress",
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

	-- #TRANSLATORS: Helptext for a militarysite: Fortress
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 103, 80 },
			fps = 10
		},
		build = {
			pictures = { dirname .. "build_\\d+.png" },
			hotspot = { 103, 80 }
		},
		unoccupied = {
			pictures = { dirname .. "unoccupied_\\d+.png" },
			hotspot = { 103, 80 }
		}
	},

	max_soldiers = 8,
   heal_per_second = 170,
   conquers = 11,
   prefer_heroes = true,

   ai_hints = {
		expansion = true,
		fighting = true,
		mountain_conqueror = true,
		prohibited_till=1800
   },

   messages = {
		occupied = _"Your soldiers have occupied your fortress.",
		aggressor = _"Your fortress discovered an aggressor.",
		attack = _"Your fortress is under attack.",
		defeated_enemy = _"The enemy defeated your soldiers at the fortress.",
		defeated_you = _"Your soldiers defeated the enemy at the fortress."
   },
}
