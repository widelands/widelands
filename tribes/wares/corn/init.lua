dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "corn",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Corn"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "corn"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 25
	},
   preciousness = {
		atlanteans = 12
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Corn
		atlanteans = pgettext("atlanteans_ware", "This corn is processed in the mill into fine cornmeal that every Atlantean baker needs for a good bread. Also horse and spider farms need to be provided with corn.")
	},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 11 },
      },
   }
}
