dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "tabard",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Tabard"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "tabards"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 30
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
