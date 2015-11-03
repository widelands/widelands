dirname = path.dirname(__file__)

tribes:new_warehouse_type {
   msgctxt = "empire_building",
   name = "empire_headquarters_shipwreck",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Headquarters Shipwreck"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "big",
   destructible = false,

   animations = {
		idle = {
			template = "idle_??",
			directory = dirname,
			hotspot = { 93, 40 },
		},
	},

	aihints = {},

	heal_per_second = 170,
	conquers = 9,
}
