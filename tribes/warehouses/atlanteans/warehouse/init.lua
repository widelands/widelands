dirname = path.dirname(__file__)

tribes:new_warehouse_type {
   name = "atlanteans_warehouse",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Warehouse",
   size = "medium",

   buildcost = {
		log = 2,
		planks = 2,
		granite = 2,
		quartz = 1,
		spidercloth = 1
	},
	return_on_dismantle = {
		log = 1,
		planks = 1,
		granite = 1,
		quartz = 1
	},

	-- TRANSLATORS: Helptext for a building: Warehouse
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 58, 62 }
		}
	},

   heal_per_second = 170,
}
