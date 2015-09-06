dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "hammer",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Hammer"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "hammers"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 2,
		barbarians = 2,
		empire = 2
	},
   preciousness = {
		atlanteans = 1,
		barbarians = 1,
		empire = 1
	},

   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 3, 4 },
      },
   }
}
