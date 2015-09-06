dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "tabard_golden",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Golden Tabard"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "golden tabards"),
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
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 10 },
      },
   }
}
