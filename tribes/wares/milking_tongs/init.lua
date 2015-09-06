dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "milking_tongs",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Milking Tongs"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "milking tongs"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 1
	},
   preciousness = {
		atlanteans = 0
	},

   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 9 },
      },
   }
}
