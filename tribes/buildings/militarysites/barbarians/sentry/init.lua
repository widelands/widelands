dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   name = "barbarians_sentry",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Sentry",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
		blackwood = 2
	},
	return_on_dismantle = {
		blackwood = 1
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
			hotspot = { 39, 40 },
			fps = 10
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 39, 40 },
		},
		unoccupied = {
			pictures = path.list_directory(dirname, "unoccupied_\\d+.png"),
			hotspot = { 39, 40 }
		}
	},

   aihints = {
		expansion = true,
		fighting = true,
		mountain_conqueror = true
   },

	outputs = {
		"barbarians_soldier",
   },

	max_soldiers = 2,
   heal_per_second = 80,
   conquers = 6,
   prefer_heroes = false,

   messages = {
		occupied = _"Your soldiers have occupied your sentry.",
		aggressor = _"Your sentry discovered an aggressor.",
		attack = _"Your sentry is under attack.",
		defeated_enemy = _"The enemy defeated your soldiers at the sentry.",
		defeated_you = _"Your soldiers defeated the enemy at the sentry."
   },
}
