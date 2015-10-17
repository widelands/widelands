dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "fish",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Fish"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 20,
		barbarians = 20,
		empire = 10
	},
   preciousness = {
		atlanteans = 4,
		barbarians = 3,
		empire = 3
	},

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 8, 6 },
      },
   }
}
