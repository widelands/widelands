dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   name = "empire_outpost",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Outpost",
   size = "medium",
   enhancement = "empire_barrier",

   buildcost = {
		log = 1,
		planks = 1,
		granite = 1,
		marble = 1
	},
	return_on_dismantle = {
		granite = 1,
		marble = 1
	},

	-- #TRANSLATORS: Helptext for a militarysite: Outpost
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 57, 77 }
		}
	},

	max_soldiers = 3,
   heal_per_second = 100,
   conquers = 7,
   prefer_heroes = false,

   aihints = {
		expansion = true,
		prohibited_till=600
   },

   messages = {
		occupied = _"Your soldiers have occupied your outpost.",
		aggressor = _"Your outpost discovered an aggressor.",
		attack = _"Your outpost is under attack.",
		defeated_enemy = _"The enemy defeated your soldiers at the outpost.",
		defeated_you = _"Your soldiers defeated the enemy at the outpost."
   },
}
