dirname = path.dirname(__file__)

tribes:new_warehouse_type {
   name = "empire_headquarters",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Headquarters",
   icon = dirname .. "menu.png",
   size = "big",
   buildable = false,
   destructible = false,

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"Text needed",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"Accommodation for your people. Also stores your wares and tools.",
		-- TRANSLATORS: Note helptext for a building
		note = _"The headquarters is your main building." .. "<br>" .. _"Text needed",
		-- #TRANSLATORS: Performance helptext for a building
		performance = ""
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 95, 109 },
		},
	},

	aihints = {},

	heal_per_second = 220,
	conquers = 9,
}
