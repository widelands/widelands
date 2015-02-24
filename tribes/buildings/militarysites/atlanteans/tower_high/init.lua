dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   name = "atlanteans_tower_high",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"High Tower",
   icon = dirname .. "menu.png",
   size = "medium",
   vision_range = 21,

   enhancement_cost = {
		log = 1,
		planks = 1,
		granite = 2
	},
	return_on_dismantle_on_enhanced = {
		granite = 1
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"Text needed",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Garrisons soldiers to expand your territory.",
		-- TRANSLATORS: Note helptext for a building
		note = _"If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.",
		-- #TRANSLATORS: Performance helptext for a building
		performance = ""
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png" ),
			hotspot = { 50, 73 },
		}
	},

	aihints = {},

	outputs = {
		"atlanteans_soldier",
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
