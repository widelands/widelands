dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "milking_tongs",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Milking Tongs"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "milking tongs"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 1
	},
   preciousness = {
		atlanteans = 0
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Milking Tongs
		atlanteans = pgettext("atlanteans_ware", "Milking tongs are used by the spider breeder to milk the spiders. They are produced by the toolsmith.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 9 },
      },
   }
}
