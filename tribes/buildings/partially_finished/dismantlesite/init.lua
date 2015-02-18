dirname = path.dirname(__file__)

tribes:new_dismantlesite_type {
   name = "dismantlesite",
   -- TRANSLATORS: This is a name used in lists of buildings for buildings being taken apart
   descname = _"Dismantle Site",
   size = "small", -- Dummy; overridden by building size
   buildable = false,
   vision_range = 2,

   helptexts = {
		-- TRANSLATORS: Lore helptext for a building
		lore = _"Text needed",
		-- TRANSLATORS: Lore author helptext for a building
		lore_author = _"Source needed",
		-- TRANSLATORS: Purpose helptext for a building
		purpose = _"A building is being dismantled at this dismantlesite, returning some of the resources that were used during this building’s construction to your tribe’s stores.",
		-- #TRANSLATORS: Note helptext for a building
		note = "",
		-- TRANSLATORS: Performance helptext for a building
		performance = _"Calculation needed"
   },

   animations = {
		idle = {
			pictures = path.list_directory(dirname, "idle_\\d+.png"),
			hotspot = { 5, 5 },
		},
	},

	aihints = {},
}
