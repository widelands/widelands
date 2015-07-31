dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "empire_well",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Well",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
		log = 2,
		granite = 1,
		marble = 1
	},
	return_on_dismantle = {
		log = 1,
		marble = 1
	},

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"Text needed",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Draws water out of the deep.",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"Calculation needed"
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 43, 43 },
		},
		working = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"), -- TODO(GunChleoc): No animation yet.
			hotspot = { 43, 43 },
		},
	},

   aihints = {
		mines_water = true,
		prohibited_till = 60
   },

	working_positions = {
		empire_carrier = 1
	},

   outputs = {
		"water"
   },

	programs = {
		work = {
			-- TRANSLATORS: Completed/Skipped/Did not start working because ...
			descname = _"working",
			actions = {
				"sleep=20000",
				"animate=working 20000",
				"mine=water 1 100 65 2",
				"produce=water"
			}
		},
	},
	out_of_resource_notification = {
		title = _"Out of Water",
		message = _"The carrier working at this well can’t find any water in his work area.",
		productivity_threshold = 33
	},
}
