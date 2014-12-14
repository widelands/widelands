dirname = path.dirname(__file__)

tribes:new_warehouse_type {
   name = "empire_warehouse",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Warehouse",
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

	-- #TRANSLATORS: Helptext for a building: Warehouse
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 58, 55 }
		}
	},

   heal_per_second = 170,
}
