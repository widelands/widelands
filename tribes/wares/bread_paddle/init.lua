dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "bread_paddle",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Bread Paddle"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "bread paddles"),
   icon = dirname .. "menu.png",
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
		-- TRANSLATORS: Helptext for a ware: Bread Paddle
		default = pgettext("default_ware", "The bread paddle is the tool of the baker, each baker needs one."),
		-- TRANSLATORS: Helptext for a ware: Bread Paddle
		atlanteans = pgettext("atlanteans_ware", "Bread paddles are produced by the toolsmith."),
		-- TRANSLATORS: Helptext for a ware: Bread Paddle
		barbarians = pgettext("barbarians_ware", "Bread paddles are produced in the metal workshop like all other tools (but cease to be produced by the building if it is enhanced to an axfactory and war mill)."),
		-- TRANSLATORS: Helptext for a ware: Bread Paddle
		empire = pgettext("empire_ware", "Bread paddles are produced by the toolsmith.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 6 },
      },
   }
}
