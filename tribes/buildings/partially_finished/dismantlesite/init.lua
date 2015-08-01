-- This include can be removed when all help texts have been defined.
include "tribes/scripting/global_helptexts.lua"

dirname = path.dirname(__file__)

tribes:new_dismantlesite_type {
   name = "dismantlesite",
   -- TRANSLATORS: This is a name used in lists of buildings for buildings being taken apart
   descname = _"Dismantle Site",
   size = "small", -- Dummy; overridden by building size
   vision_range = 2,

   helptexts = {
		-- #TRANSLATORS: Lore helptext for a building
		lore = no_lore_text_yet(),
		-- #TRANSLATORS: Lore author helptext for a building
		lore_author = no_lore_author_text_yet(),
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"A building is being dismantled at this dismantlesite, returning some of the resources that were used during this building’s construction to your tribe’s stores.",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- #TRANSLATORS: Performance helptext for a building
		performance = no_performance_text_yet()
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 5, 5 },
		},
	},

	aihints = {},
}
