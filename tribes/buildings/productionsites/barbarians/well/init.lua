dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   name = "barbarians_well",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Well",
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
		log = 4
	},
	return_on_dismantle = {
		log = 2
	},

   helptexts = {
		-- #TRANSLATORS: Lore helptext for a building
		lore = _[[‘Oh how sweet is the source of life,<br> that comes down from the sky <br> and lets the earth drink.’]],
		-- #TRANSLATORS: Lore author helptext for a building
		lore_author = _[[Song written by Sigurd the Bard<br>when the first rain fell after the Great Drought in the 21st year of Chat’Karuth’s reign.]],
		-- #TRANSLATORS: Purpose helptext for a building
		purpose = _"Draws water out of the deep.",
		-- #TRANSLATORS: Note helptext for a building
		note = _"",
		-- #TRANSLATORS: Performance helptext for a building
		performance = _"The carrier needs %s to get one bucket full of water.":bformat(
		ngettext("%d second", "%d seconds", 40):bformat(40))
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 19, 33 },
		},
		build = {
			pictures = path.list_directory(dirname, "build_\\d+.png"),
			hotspot = { 19, 33 },
			fps = 1
		},
		working = {
			pictures = path.list_directory(dirname, "working_\\d+.png"),
			hotspot = { 19, 33 },
			fps = 1
		},
	},

   aihints = {
		mines_water = true,
		prohibited_till = 600
   },

	working_positions = {
		barbarians_carrier = 1
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
				"produce=water",
			}
		},
	},

	out_of_resource_notification = {
		title = _"Out of Water",
		message = _"The carrier working at this well can’t find any water in his working radius.",
		delay_attempts = 50
	},
}
