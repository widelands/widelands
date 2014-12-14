dirname = path.dirname(__file__)

tribes:new_warehouse_type {
   name = "empire_port",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Port",
   size = "port",

   buildcost = {
		log = 3,
		planks = 4,
		granite = 4,
		marble = 2,
		marble_column = 1,
		cloth = 3,
		gold = 2
	},
	return_on_dismantle = {
		log = 1,
		planks = 1,
		granite = 2,
		marble = 2,
		cloth = 1,
		gold = 1
	},

	-- TRANSLATORS: Helptext for a building: Port
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 74, 96 },
			fps = 10
		},
		build = {
			pictures = { dirname .. "build_\\d+.png" },
			hotspot = { 74, 96 }
		}
	},

   conquers = 5,
   heal_per_second = 170,
}
