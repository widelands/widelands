dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "trident_heavy_double",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Heavy Double Trident"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "heavy double tridents"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 1
	},
   preciousness = {
		atlanteans = 1
	},

   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 17 },
      },
   }
}
