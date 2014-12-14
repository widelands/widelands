dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   name = "atlanteans_tower_high",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"High Tower",
   size = "medium",
   buildable = false
   vision_range=21,
   enhanced_building = true,

   enhancement_cost = {
		log = 1,
		planks = 1,
		granite = 2
	},
	return_on_dismantle_on_enhanced = {
		granite = 1
	},

	-- #TRANSLATORS: Helptext for a militarysite: High Tower
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 50, 73 },
		}
	},

   max_soldiers = 5,
   heal_per_second = 170,
   conquers = 9,
   prefer_heroes = true,

   messages = {
		occupied = _"Your soldiers have occupied your high tower.",
		aggressor = _"Your high tower discovered an aggressor.",
		attack = _"Your high tower is under attack.",
		defeated_enemy = _"The enemy defeated your soldiers at the high tower.",
		defeated_you = _"Your soldiers defeated the enemy at the high tower."
   },
}
