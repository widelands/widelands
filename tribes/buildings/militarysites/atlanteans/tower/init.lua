-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   name = "atlanteans_tower",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Tower",
   icon = dirname .. "menu.png",
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

   helptexts = {
		-- #TRANSLATORS: Lore helptext for a building
		lore = no_lore_text_yet(),
		-- #TRANSLATORS: Lore author helptext for a building
		lore_author = no_lore_author_text_yet(),
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Garrisons soldiers to expand your territory.",
		-- TRANSLATORS: Note helptext for a building
		note = _"If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.",
		-- #TRANSLATORS: Performance helptext for a building
		performance = ""
   },

   aihints = {
		expansion = true,
		mountain_conqueror = true,
		prohibited_till = 600
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png" ),
			hotspot = { 55, 60 },
		}
	},

	outputs = {
		"atlanteans_soldier",
   },

   max_soldiers = 4,
   heal_per_second = 120,
   conquers = 9,
   prefer_heroes = false,

   messages = {
		occupied = _"Your soldiers have occupied your tower.",
		aggressor = _"Your tower discovered an aggressor.",
		attack = _"Your tower is under attack.",
		defeated_enemy = _"The enemy defeated your soldiers at the tower.",
		defeated_you = _"Your soldiers defeated the enemy at the tower."
   },
}
