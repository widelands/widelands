-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   name = "empire_castle",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Castle",
   icon = dirname .. "menu.png",
   size = "big",

   enhancement_cost = {
		planks = 5,
		marble_column = 4,
		marble = 4,
		granite = 2
	},
	return_on_dismantle_on_enhanced = {
		planks = 2,
		marble_column = 2,
		marble = 3,
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
			hotspot = { 94, 106 }
		}
	},

	aihints = {},

	outputs = {
		"empire_soldier",
   },

	max_soldiers = 12,
   heal_per_second = 220,
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
