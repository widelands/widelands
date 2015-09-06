dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "empire_ware",
   name = "empire_bread",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("empire_ware", "Bread"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("empire_ware", "bread"),
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
         hotspot = { 6, 6 },
      },
   }
}
