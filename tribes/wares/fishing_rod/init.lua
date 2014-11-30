dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "fishing_rod",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Fishing Rod",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"fishing rods",
   default_target_quantity = {
		barbarians = 1,
		empire = 1
	},
   preciousness = {
		barbarians = 0,
		empire = 0
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Fishing Rod
		default = _"Fishing rods are needed by fishers to catch fish.",
		-- TRANSLATORS: Helptext for a ware: Fishing Rod
		barbarians = _"They are one of the basic tools produced in a metal workshop (but cease to be produced by the building if it is enhanced to an axfactory and war mill).",
		-- TRANSLATORS: Helptext for a ware: Fishing Rod
		empire = _"They are produced by the toolsmith."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 5 },
      },
   }
}
