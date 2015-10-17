dirname = path.dirname(__file__)

tribes:new_warehouse_type {
   msgctxt = "empire_building",
   name = "empire_warehouse",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Warehouse"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
		log = 2,
		planks = 2,
		granite = 2,
		marble = 3,
		marble_column = 2
	},
	return_on_dismantle = {
		planks = 1,
		granite = 2,
		marble = 2,
		marble_column = 1
	},

   animations = {
		idle = {
			template = "idle_??",
			directory = dirname,
			hotspot = { 58, 55 }
		}
	},

	aihints = {},

   heal_per_second = 170,
}
