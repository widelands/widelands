dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "shield_steel",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Steel Shield"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "steel shields"),
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
         hotspot = { 10, 17 },
      },
   }
}
