dirname = path.dirname(__file__)

tribes:new_warehouse_type {
   name = "empire_headquarters_shipwreck",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Headquarters Shipwreck",
   size = "big",
   buildable = false,
   destructible = false,

	-- #TRANSLATORS: Helptext for a building: Headquarters
   helptext = "", -- NOCOM(GunChleoc): See what we can shift over from help.lua here

   animations = {
		idle = {
			pictures = { dirname .. "idle_\\d+.png" },
			hotspot = { 93, 40 },
		},
	},

	heal_per_second = 170,
	conquers = 9,
}
