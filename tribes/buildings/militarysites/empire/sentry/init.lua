dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   name = "empire_sentry",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Sentry",
   size = "small",
   enhanced_building = true,

   buildcost = {
		planks = 1,
		log = 1,
		granite = 1
	},
	return_on_dismantle = {
		granite = 1,
		planks = 1
	},
   enhancement_cost = {
		planks = 1,
		granite = 1
	},
	return_on_dismantle_on_enhanced = {
		granite = 1
	},

	-- #TRANSLATORS: Helptext for a militarysite: Sentry
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 37, 60 }
		}
	},

	max_soldiers = 2,
   heal_per_second = 80,
   conquers = 6,
   prefer_heroes = false,

   aihints = {
		expansion = true,
		fighting = true,
		mountain_conqueror = true
   },

   messages = {
		occupied = _"Your soldiers have occupied your sentry.",
		aggressor = _"Your sentry discovered an aggressor.",
		attack = _"Your sentry is under attack.",
		defeated_enemy = _"The enemy defeated your soldiers at the sentry.",
		defeated_you = _"Your soldiers defeated the enemy at the sentry."
   },
}
