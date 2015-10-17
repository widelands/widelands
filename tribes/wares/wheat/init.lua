dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "wheat",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Wheat"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
		barbarians = 25,
		empire = 25
	},
   preciousness = {
		barbarians = 12,
		empire = 12
	},

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { -1, 6 },
      },
   }
}
