dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "shovel",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Shovel",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"shovels",
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
		default = _"Shovels are needed for the proper handling of plants.",
		-- TRANSLATORS: Helptext for a ware: Shovel
		atlanteans = _"Therefore the forester and the blackroot farmer use them. They are produced by the toolsmith.",
		-- TRANSLATORS: Helptext for a ware: Shovel
		barbarians = _"Therefore the gardener and the ranger use them. Produced at the metal workshop (but cease to be produced by the building if it is enhanced to an axfactory and war mill).",
		-- TRANSLATORS: Helptext for a ware: Shovel
		empire = _"Therefore the forester and the vinefarmer use them. They are produced by the toolsmith."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 5 },
      },
   }
}
