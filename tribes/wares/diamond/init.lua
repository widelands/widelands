dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "diamond",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Diamond"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "diamonds"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 5
	},
   preciousness = {
		atlanteans = 2
	},

   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 7 },
      },
   }
}
