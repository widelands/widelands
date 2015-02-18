dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_micro_brewery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Micro Brewery",
   icon = dirname .. "menu.png",
   size = "medium",
   enhancement = "barbarians_brewery",

   buildcost = {
		log = 3,
		blackwood = 2,
		granite = 3,
		thatch_reed = 2
	},
	return_on_dismantle = {
		log = 1,
		blackwood = 1,
		granite = 2
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"‘Let the first one drive away the hunger, the second one put you at ease; when you have swallowed up your third one, it’s time for the next shift!’",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Widespread toast among Miners",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"The micro brewery produces beer of the lower grade. This beer is a vital component of the snacks that inns and big inns prepare for miners in deep mines.",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"If all needed wares are delivered in time, this building can produce beer in about %s on average.":bformat(ngettext("%d second", "%d seconds", 60):bformat(60))
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 42, 50 },
		},
		working = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"), -- TODO(GunChleoc): No animation yet.
			hotspot = { 42, 50 },
		},
	},

   aihints = {
		forced_after = 500
   },

	working_positions = {
		barbarians_brewer = 1
	},

   inputs = {
		water = 8,
		wheat = 8
	},
   outputs = {
		"beer"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start brewing beer because ...
			descname = _"brewing beer",
			actions = {
				"sleep=30000",
				"return=skipped unless economy needs beer or workers need experience",
				"consume=water wheat",
				"animate=working 30000",
				"produce=beer"
			}
		},
	},
}
