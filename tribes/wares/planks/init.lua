dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "planks",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Planks"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 40,
		empire = 40
	},
   preciousness = {
		atlanteans = 10,
		empire = 10
	},

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 12, 17 },
      },
   }
}
