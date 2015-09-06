dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "hunting_bow",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Hunting Bow"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "hunting bows"),
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
         hotspot = { 9, 16 },
      },
   }
}
