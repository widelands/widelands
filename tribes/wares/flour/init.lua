dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "flour",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Flour"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "flour"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
		empire = 20
	},
   preciousness = {
		empire = 5
	},

   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 15 },
      },
   }
}
