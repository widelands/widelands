dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "tabard_golden",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Golden Tabard"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 1
	},
   preciousness = {
		atlanteans = 1
	},

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 5, 10 },
      },
   }
}
