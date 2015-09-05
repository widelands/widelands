dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "fire_tongs",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Fire Tongs"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "fire tongs"),
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
		-- TRANSLATORS: Helptext for a ware: Fire Tongs
		default = pgettext("default_ware", "Fire tongs are the tools of the smelter, who works in the smelting works."),
		-- TRANSLATORS: Helptext for a ware: Fire Tongs
		atlanteans = pgettext("atlanteans_ware", "They are produced by the toolsmith."),
		-- TRANSLATORS: Helptext for a ware: Fire Tongs
		barbarians = pgettext("barbarians_ware", "Produced by the metal works (but they cease to be produced by the building if it is enhanced to an axfactory and war mill)."),
		-- TRANSLATORS: Helptext for a ware: Fire Tongs
		empire = pgettext("empire_ware", "They are produced by the toolsmith.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 5 },
      },
   }
}
