dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "fishing_net",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Fishing Net"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "fishing nets"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 2
	},
   preciousness = {
		atlanteans = 0
	},

   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 17 },
      },
   }
}
