dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   name = "empire_blockhouse",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Blockhouse",
   size = "small",
   enhancement = "empire_sentry",

   buildcost = {
		log = 1,
		planks = 2
	},
	return_on_dismantle = {
		planks = 1
	},

	-- #TRANSLATORS: Helptext for a militarysite: Blockhouse
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 37, 58 }
		}
	},

	max_soldiers = 1,
   heal_per_second = 60, -- very low -> smallest building
   conquers = 6,
   prefer_heroes = false,

   messages = {
		occupied = _"Your soldiers have occupied your blockhouse.",
		aggressor = _"Your blockhouse discovered an aggressor.",
		attack = _"Your blockhouse is under attack.",
		defeated_enemy = _"The enemy defeated your soldiers at the blockhouse.",
		defeated_you = _"Your soldiers defeated the enemy at the blockhouse."
   },
}
