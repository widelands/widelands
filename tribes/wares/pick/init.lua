dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "pick",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Pick"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "picks"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 3,
		barbarians = 2,
		empire = 2
	},
   preciousness = {
		atlanteans = 1,
		barbarians = 1,
		empire = 1
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Pick
		atlanteans = pgettext("atlanteans_ware", "Picks are used by stonecutters and miners. They are produced by the toolsmith."),
		-- TRANSLATORS: Helptext for a ware: Pick
		barbarians = pgettext("barbarians_ware", "Picks are used by stonemasons and miners. They are produced in the metal workshop (but cease to be produced by the building if it is enhanced to an axfactory and war mill)."),
		-- TRANSLATORS: Helptext for a ware: Pick
		empire = pgettext("empire_ware", "Picks are used by stonemasons and miners. They are produced by the toolsmith.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 4 },
      },
   }
}
