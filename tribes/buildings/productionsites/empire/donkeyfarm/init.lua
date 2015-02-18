dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "empire_donkeyfarm",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Donkey Farm",
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
		purpose = _"Breeds cute and helpful donkeys for adding them to the transportation system.",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"Calculation needed"
   }

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 85, 78 },
		},
		working = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"), -- TODO(GunChleoc): No animation yet.
			hotspot = { 85, 78 },
		},
	},

   aihints = {
		recruitment = true
   },

	working_positions = {
		empire_donkeybreeder = 1
	},

   inputs = {
		wheat = 8,
		water = 8
	},
   outputs = {
		"empire_donkey"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start rearing donkeys because ...
			descname = _"rearing donkeys",
			actions = {
				"sleep=15000",
				"return=skipped unless economy needs empire_donkey",
				"consume=wheat water",
				"playFX=../../../sound/farm/donkey 192",
				"animate=working 15000", -- Feeding cute little baby donkeys ;)
				"recruit=empire_donkey"
			}
		},
	},
}
