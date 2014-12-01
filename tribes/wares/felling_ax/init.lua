dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "felling_ax",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Felling Ax",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"felling axes",
   default_target_quantity = {
		barbarians = 5,
		empire = 3
	},
   preciousness = {
		barbarians = 3,
		empire = 1
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Felling Ax
		default = _"The felling ax is the tool of the lumberjack to chop down trees.",
		-- TRANSLATORS: Helptext for a ware: Felling Ax
		barbarians = _"Felling axes are produced in the metal workshop (but cease to be produced by the building if it is enhanced to an axfactory and war mill).",
		-- TRANSLATORS: Helptext for a ware: Felling Ax
		empire = _"Felling axes are produced by the toolsmith."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 7 },
      },
   }
}
