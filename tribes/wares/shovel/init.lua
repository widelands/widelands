dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "shovel",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Shovel"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "shovels"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 2,
		barbarians = 1,
		empire = 1,
	},
   preciousness = {
		atlanteans = 0,
		barbarians = 0,
		empire = 0
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Shovel
		default = pgettext("default_ware", "Shovels are needed for the proper handling of plants."),
		-- TRANSLATORS: Helptext for a ware: Shovel
		atlanteans = pgettext("atlanteans_ware", "Therefore the forester and the blackroot farmer use them. They are produced by the toolsmith."),
		-- TRANSLATORS: Helptext for a ware: Shovel
		barbarians = pgettext("barbarians_ware", "Therefore the gardener and the ranger use them. Produced at the metal workshop (but cease to be produced by the building if it is enhanced to an axfactory and war mill)."),
		-- TRANSLATORS: Helptext for a ware: Shovel
		empire = pgettext("empire_ware", "Therefore the forester and the vinefarmer use them. They are produced by the toolsmith.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 5 },
      },
   }
}
