dirname = path.dirname(__file__)

tribes:new_warehouse_type {
   name = "barbarians_warehouse",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Warehouse",
   size = "medium",

   buildcost = {
		log = 3,
		blackwood = 2,
		granite = 2,
		grout = 3,
		thatch_reed = 1
	},
	return_on_dismantle = {
		log = 1,
		blackwood = 1,
		granite = 1,
		grout = 1
	},

	-- #TRANSLATORS: Helptext for a building: Warehouse
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 58, 58 }
		},
		build = {
			pictures = { dirname .. "build_\\d+.png" },
			hotspot = { 58, 58 }
		}
	},

   heal_per_second = 170,
}
