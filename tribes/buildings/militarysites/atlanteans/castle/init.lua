dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   name = "atlanteans_castle",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Castle",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
		planks = 4,
		granite = 7,
		log = 5,
		spidercloth = 2,
		diamond = 1,
		quartz = 1
	},
	return_on_dismantle = {
		planks = 2,
		granite = 5,
		log = 2
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
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 91, 91 },
		}
	},

   aihints = {
		expansion = true,
		fighting = true,
		mountain_conqueror = true,
		prohibited_till = 1500
   },

	outputs = {
		"atlanteans_soldier",
   },

   max_soldiers = 12,
   heal_per_second = 200,
   conquers = 12,
   prefer_heroes = true,

   messages = {
		occupied = _"Your soldiers have occupied your castle.",
		aggressor = _"Your castle discovered an aggressor.",
		attack = _"Your castle is under attack.",
		defeated_enemy = _"The enemy defeated your soldiers at the castle.",
		defeated_you = _"Your soldiers defeated the enemy at the castle."
   },
}
