dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "buckets",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Buckets"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "buckets"),
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
