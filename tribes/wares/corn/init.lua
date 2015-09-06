dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "corn",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Corn"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "corn"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 25
	},
   preciousness = {
		atlanteans = 12
	},

   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 11 },
      },
   }
}
