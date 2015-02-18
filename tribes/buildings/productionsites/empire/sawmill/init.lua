dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "empire_sawmill",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Sawmill",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 2,
		granite = 1,
		planks = 1
	},
	return_on_dismantle = {
		log = 1,
		granite = 1
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"Text needed",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Text needed",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"Calculation needed"
   }

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 54, 70 },
		},
		working = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"), -- TODO(GunChleoc): No animation yet.
			hotspot = { 54, 70 },
		},
	},

   aihints = {
		forced_after = 60,
		prohibited_till = 60
   },

	working_positions = {
		empire_carpenter = 1
	},

   inputs = {
		log = 8
	},
   outputs = {
		"planks"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start sawing logs because ...
			descname = _"sawing logs",
			actions = {
				"sleep=16500", -- Much faster than barbarians' wood hardener
				"return=skipped unless economy needs planks",
				"consume=log:2",
				"playFX=../../../sound/sawmill/sawmill 180",
				"animate=working 20000", -- Much faster than barbarians' wood hardener
				"produce=planks"
			}
		},
	},
}
