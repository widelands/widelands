-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_warehouse_type {
   name = "atlanteans_headquarters",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = _"Headquarters",
   size = "big",
   destructible = false,

   helptexts = {
		-- #TRANSLATORS: Lore helptext for a building
		lore = no_lore_text_yet(),
		-- #TRANSLATORS: Lore author helptext for a building
		lore_author = no_lore_author_text_yet(),
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
			hotspot = { 81, 110 },
		},
	},

	aihints = {},

	heal_per_second = 220,
	conquers = 9,
}
