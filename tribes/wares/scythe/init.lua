dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "scythe",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Scythe",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"scythes",
   default_target_quantity = {
		atlanteans = 1,
		barbarians = 1,
		empire = 1
	},
   preciousness = {
		atlanteans = 0,
		barbarians = 0,
		empire = 0
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Scythe
		default = _"The scythe is the tool of the farmers.",
		-- TRANSLATORS: Helptext for a ware: Scythe
		atlanteans = _"Scythes are produced by the toolsmith.",
		-- TRANSLATORS: Helptext for a ware: Scythe
		barbarians = _" Scythes are produced by the metal workshop (but cease to be produced by the building if it is enhanced to an axfactory and war mill).",
		-- TRANSLATORS: Helptext for a ware: Scythe
		empire = _"Scythes are produced by the toolsmith."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 2 },
      },
   }
}
