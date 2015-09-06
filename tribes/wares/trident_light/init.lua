dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "trident_light",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Light Trident"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "light tridents"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 30
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
