dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "empire_charcoal_kiln",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Charcoal Kiln",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 2,
		granite = 2,
		marble = 2
	},
	return_on_dismantle = {
		log = 1,
		granite = 1,
		marble = 1
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
			hotspot = { 51, 48 },
		},
		working = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"), -- TODO(GunChleoc): No animation yet.
			hotspot = { 51, 48 },
		},
	},

   aihints = {
		prohibited_till = 600
   },

	working_positions = {
		empire_charcoal_burner = 1
	},

   inputs = {
		log = 8
	},
   outputs = {
		"coal"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start producing coal because ...
			descname = _"producing coal",
			actions = {
				"sleep=30000",
				"return=skipped unless economy needs coal",
				"consume=log:6",
				"animate=working 90000", -- Charcoal fires will burn for some days in real life
				"produce=coal"
			}
		},
	},
}
