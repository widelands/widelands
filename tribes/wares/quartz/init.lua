dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "quartz",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Quartz"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "quartz"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 5
	},
   preciousness = {
		atlanteans = 1
	},

   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 13 },
      },
   }
}
