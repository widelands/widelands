dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_bakery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Bakery",
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 2,
		blackwood = 2,
		granite = 2,
		thatch_reed = 2
	},
	return_on_dismantle = {
		log = 1,
		blackwood = 1,
		granite = 2
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"Text needed",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Bakes pitta bread for soldiers and miners alike.",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"If all needed wares are delivered in time, this building can produce a pitta bread in %s on average.":bformat(ngettext("%d second", "%d seconds", 30):bformat(30))
   }

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 41, 58 },
		},
		unoccupied = {
			pictures = path.list_directory(dirname, "unoccupied_\\d+.png"),
			hotspot = { 41, 58 },
		},
		working = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"), -- TODO(GunChleoc): No animation yet.
			hotspot = { 41, 58 },
		},
	},

   aihints = {
		prohibited_till = 600
   },

	working_positions = {
		barbarians_baker = 1
	},

   inputs = {
		wheat = 6,
		water = 6
	},
   outputs = {
		"bread_barbarians"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start baking bread because ...
			descname = _"baking bread",
			actions = {
				"sleep=20000",
				"return=skipped unless economy needs bread_barbarians",
				"consume=water:3 wheat:3",
				"animate=working 20000",
				"produce=bread_barbarians",
				"animate=working 20000",
				"produce=bread_barbarians"
			}
		},
	},
}
