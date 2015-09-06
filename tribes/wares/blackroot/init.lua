dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "blackroot",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Blackroot"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "blackroots"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 20
	},
   preciousness = {
		atlanteans = 10
	},

   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 9, 15 },
      },
   }
}
