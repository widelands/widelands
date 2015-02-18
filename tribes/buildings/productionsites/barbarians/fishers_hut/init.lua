dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_fishers_hut",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Fisher’s Hut",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
		log = 4
	},
	return_on_dismantle = {
		log = 2
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"‘What do you mean, my fish ain’t fresh?!’",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Fishes on the coast near the hut.",
		-- TRANSLATORS: Note helptext for a building
		note = _"The fisher’s hut needs water full of fish within the working radius.",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"The fisher pauses %s before going to work again.":bformat(ngettext("%d second", "%d seconds", 18):bformat(18))
   }

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 39, 40 },
		},
	},

   aihints = {
		needs_water = true,
		prohibited_till = 900
   },

	working_positions = {
		barbarians_fisher = 1
	},

   outputs = {
		"fish"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start fishing because ...
			descname = _"fishing",
			actions = {
				"sleep=18000",
				"worker=fish"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Out of Fish",
		message = _"The fisher working out of this fisher’s hut can’t find any fish in his working radius.",
		delay_attempts = 0
	},
}
