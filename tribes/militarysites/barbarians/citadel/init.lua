dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   name = "barbarians_citadel",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Citadel",
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

	-- TRANSLATORS: Helptext for a militarysite: Citadel
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 102, 102 },
			fps = 10
		},
		build = {
			pictures = { dirname .. "build_\\d+.png" },
			hotspot = { 102, 102 }
		},
		unoccupied = {
			pictures = { dirname .. "unoccupied_\\d+.png" },
			hotspot = { 102, 102 }
		}
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
