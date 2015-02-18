dirname = path.dirname(__file__)

tribes:new_warehouse_type {
   name = "empire_headquarters_shipwreck",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Headquarters Shipwreck",
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
		purpose = _"Although this ship ran aground, it still serves as accommodation for your people. It also stores your wares and tools.",
		-- TRANSLATORS: Note helptext for a building
		note = _"The headquarters shipwreck is your main building." .. "<br>" .. _"Text needed",
		-- #TRANSLATORS: Performance helptext for a building
		performance = ""
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 93, 40 },
		},
	},

	aihints = {},

	heal_per_second = 170,
	conquers = 9,
}
