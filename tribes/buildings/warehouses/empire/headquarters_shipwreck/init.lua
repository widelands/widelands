-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_warehouse_type {
   name = "empire_headquarters_shipwreck",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Headquarters Shipwreck",
   size = "big",
   destructible = false,

   helptexts = {
		-- #TRANSLATORS: Lore helptext for a building
		lore = no_lore_text_yet(),
		-- #TRANSLATORS: Lore author helptext for a building
		lore_author = no_lore_author_text_yet(),
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
