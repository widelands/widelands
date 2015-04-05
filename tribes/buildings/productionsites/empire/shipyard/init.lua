dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "empire_shipvard",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Shipyard",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 3,
		planks = 2,
		granite = 3,
		cloth = 2
	},
	return_on_dismantle = {
		log = 1,
		granite = 2,
		cloth = 1
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
			hotspot = { 50, 63 },
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 50, 63 },
		},
		unoccupied = {
			pictures = path.list_directory(dirname, "unoccupied_\\d+.png"),
			hotspot = { 50, 63 },
		},
		working = {
			pictures = path.list_directory(dirname, "working_\\d+.png"),
			hotspot = { 50, 63 },
		},
	},

   aihints = {
		needs_water = true,
		shipyard = true,
		prohibited_till = 1500
   },

	working_positions = {
		empire_shipwright = 1
	},

   inputs = {
		planks = 10,
		log = 2,
		cloth = 4
	},
   outputs = {
		"beer"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"sleep=20000",
				"call=ship",
				"return=skipped"
			}
		},
		ship = {
			-- TRANSLATORS: Completed/Skipped/Did not start constructing a ship because ...
			descname = _"constructing a ship",
			actions = {
				"check_map=seafaring",
				"construct=empire_shipconstruction buildship 6",
				"animate=working 35000",
				"return=completed"
			}
		},
	},
}
