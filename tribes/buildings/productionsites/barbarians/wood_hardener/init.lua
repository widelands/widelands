dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_wood_hardener",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Wood Hardener",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 3,
		granite = 1
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
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 38, 50 },
		},
		working = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"), -- TODO(GunChleoc): No animation yet.
			hotspot = { 38, 50 },
		},
	},

   aihints = {
		forced_after = 0
   },

	working_positions = {
		barbarians_lumberjack = 1
	},

   inputs = {
		log = 8
	},
   outputs = {
		"blackwood"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start hardening wood because ...
			descname = _"hardening wood",
			actions = {
				"sleep=43000",
				"return=skipped unless economy needs blackwood",
				"consume=log:2",
				"animate=working 24000",
				"produce=blackwood"
			}
		},
	},
}
