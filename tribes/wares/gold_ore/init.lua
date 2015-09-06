dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "gold_ore",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Gold Ore"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "gold ore"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 15,
		barbarians = 15,
		empire = 15
	},
   preciousness = {
		atlanteans = 2,
		barbarians = 2,
		empire = 2
	},

   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 4 },
      },
   }
}
