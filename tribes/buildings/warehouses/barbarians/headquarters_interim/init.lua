dirname = path.dirname(__file__)

tribes:new_warehouse_type {
   name = "barbarians_headquarters_interim",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Headquarters",
   size = "big",
   buildable = false,
   destructible = false,

	-- #TRANSLATORS: Helptext for a building: Headquarters
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here


	-- The interim headquarters of the barbarians is a simple stone building thatched
	-- with reed and coated with many different wares.
   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 64, 106 }
		},
	},

	heal_per_second = 170,
	conquers = 9,
}
