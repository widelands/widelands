dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "basket",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Basket"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "baskets"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
		empire = 1
	},
   preciousness = {
		empire = 0
	},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 8, 11 },
      },
   }
}
