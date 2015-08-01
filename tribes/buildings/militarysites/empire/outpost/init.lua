-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   name = "empire_outpost",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Outpost",
   icon = dirname .. "menu.png",
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
			hotspot = { 57, 77 }
		}
	},

   aihints = {
		expansion = true,
		prohibited_till=600
   },

	outputs = {
		"empire_soldier",
   },

	max_soldiers = 3,
   heal_per_second = 100,
   conquers = 7,
   prefer_heroes = false,

   messages = {
		occupied = _"Your soldiers have occupied your outpost.",
		aggressor = _"Your outpost discovered an aggressor.",
		attack = _"Your outpost is under attack.",
		defeated_enemy = _"The enemy defeated your soldiers at the outpost.",
		defeated_you = _"Your soldiers defeated the enemy at the outpost."
   },
}
