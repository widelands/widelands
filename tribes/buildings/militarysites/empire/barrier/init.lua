-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   name = "empire_barrier",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Barrier",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 2,
		planks = 2,
		granite = 2,
		marble = 1
	},
	return_on_dismantle = {
		log = 1,
		planks = 1,
		granite = 1
	},
   enhancement_cost = {
		log = 1,
		planks = 2,
		granite = 1,
		marble = 1
	},
	return_on_dismantle_on_enhanced = {
		planks = 1,
		granite = 1
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

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 49, 77 }
		}
	},

   aihints = {
		fighting = true
   },

	outputs = {
		"empire_soldier",
   },

	max_soldiers = 5,
   heal_per_second = 130,
   conquers = 8,
   prefer_heroes = true,

   messages = {
		occupied = _"Your soldiers have occupied your barrier.",
		aggressor = _"Your barrier discovered an aggressor.",
		attack = _"Your barrier is under attack.",
		defeated_enemy = _"The enemy defeated your soldiers at the barrier.",
		defeated_you = _"Your soldiers defeated the enemy at the barrier."
   },
}
