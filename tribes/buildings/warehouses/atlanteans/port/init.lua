dirname = path.dirname(__file__)

tribes:new_warehouse_type {
   name = "atlanteans_port",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Port",
   size = "port",

   buildcost = {
		log = 3,
		planks = 3,
		granite = 4,
		diamond = 1,
		quartz = 1,
		spidercloth = 3,
		gold = 2
	},
	return_on_dismantle = {
		log = 1,
		planks = 1,
		granite = 2,
		spidercloth = 1,
		gold = 1
	},

	-- #TRANSLATORS: Helptext for a building: Port
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 74, 70 },
			fps = 10
		},
		build = {
			pictures = { dirname .. "build_\\d+.png" },
			hotspot = { 74, 70 }
		}
	},

   conquers = 5,
   heal_per_second = 170,
}
