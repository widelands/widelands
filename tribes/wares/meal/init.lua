dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "meal",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Meal"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "meal"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
		barbarians = 10,
		empire = 20
	},
   preciousness = {
		barbarians = 5,
		empire = 4
	},

   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 6 },
      },
   }
}
