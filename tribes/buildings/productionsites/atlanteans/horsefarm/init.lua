dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "atlanteans_horsefarm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Horse Farm",
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
		log = 2,
		granite = 2,
		planks = 1
	},
	return_on_dismantle = {
		log = 1,
		granite = 2
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"Text needed",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Breeds the strong Atlantean horses for adding them to the transportation system.",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"Calculation needed"
   }

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 81, 62 },
		},
		working = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"), -- TODO(GunChleoc): No animation yet.
			hotspot = { 81, 62 },
		}
	},

   aihints = {
		recruitment = true
   },

	working_positions = {
		atlanteans_horsebreeder = 1
	},

   inputs = {
		corn = 8,
		water = 8
	},
   outputs = {
		"atlanteans_horse"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start breeding horses because ...
			descname = _"breeding horses",
			actions = {
				"sleep=15000",
				"return=skipped unless economy needs atlanteans_horse",
				"consume=corn water",
				"playFX=../../../sound/farm/horse 192",
				"animate=working 15000", -- Feeding cute little foals ;)
				"recruit=atlanteans_horse"
			}
		},

	},
}
