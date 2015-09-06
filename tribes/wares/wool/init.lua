dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "wool",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Wool"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "wool"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
		empire = 10
	},
   preciousness = {
		empire = 2
	},

   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 8, 18 },
      },
   }
}
