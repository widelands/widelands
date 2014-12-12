dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   name = "atlanteans_tower",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Tower",
   size = "medium",
   vision_range = 17,
   enhancement = "atlanteans_tower_high",



   buildcost = {
		log = 2,
		planks = 3,
		granite = 4,
		spidercloth = 1
	},
	return_on_dismantle = {
		log = 1,
		planks = 1,
		granite = 3
	},

	-- TRANSLATORS: Helptext for a militarysite: Tower
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 55, 60 },
		}
	},

   max_soldiers = 4,
   heal_per_second = 120,
   conquers = 9,
   prefer_heroes = false,

   aihints = {
		expansion = true,
		mountain_conqueror = true,
		prohibited_till = 600
   },

   messages = {
		occupied = _"Your soldiers have occupied your tower.",
		aggressor = _"Your tower discovered an aggressor.",
		attack = _"Your tower is under attack.",
		defeated_enemy = _"The enemy defeated your soldiers at the tower.",
		defeated_you = _"Your soldiers defeated the enemy at the tower."
   },
}
