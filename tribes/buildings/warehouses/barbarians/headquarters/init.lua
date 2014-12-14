dirname = path.dirname(__file__)

tribes:new_warehouse_type {
   name = "barbarians_headquarters",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Headquarters",
   size = "big",
   buildable = false,
   destructible = false,

	-- #TRANSLATORS: Helptext for a building: Headquarters
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here


	-- The Headquarters of the barbarians is from the apperance a stable military
	-- Building, fortified base and some towers made of blackwood. Some flags
	-- in the player colors may be present
   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 90, 96 },
			fps = 10
		},
	},

	heal_per_second = 220,
	conquers = 9,
}
