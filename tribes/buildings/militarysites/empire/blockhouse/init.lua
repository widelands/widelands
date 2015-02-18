dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   name = "empire_blockhouse",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Blockhouse",
   icon = dirname .. "menu.png",
   size = "small",
   enhancement = "empire_sentry",

   buildcost = {
		log = 1,
		planks = 2
	},
	return_on_dismantle = {
		planks = 1
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
			hotspot = { 37, 58 }
		}
	},

	aihints = {},

	outputs = {
		"empire_soldier",
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
