-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_cattlefarm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Cattle Farm",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
		log = 2,
		granite = 2,
		blackwood = 1
	},
	return_on_dismantle = {
		granite = 1,
		blackwood = 1
	},

   helptexts = {
		-- #TRANSLATORS: Lore helptext for a building
		lore = no_lore_text_yet(),
		-- #TRANSLATORS: Lore author helptext for a building
		lore_author = no_lore_author_text_yet(),
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Breeds strong oxen for adding them to the transportation system.",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"If all needed wares are delivered in time, this building can produce an ox in %s on average.":bformat(ngettext("%d second", "%d seconds", 30):bformat(30))
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 57, 80 },
		},
		working = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"), -- TODO(GunChleoc): No animation yet.
			hotspot = { 57, 80 },
		},
	},

   aihints = {
		recruitment = true
   },

	working_positions = {
		barbarians_cattlebreeder = 1
	},

   inputs = {
		wheat = 8,
		water = 8
	},
   outputs = {
		"barbarians_ox"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start rearing cattle because ...
			descname = _"rearing cattle",
			actions = {
				"sleep=15000",
				"return=skipped unless economy needs barbarians_ox",
				"consume=wheat water",
				"playFX=sound/farm/ox 192",
				"animate=working 15000", -- Animation of feeding the cattle
				"recruit=barbarians_ox"
			}
		},
	},
}
