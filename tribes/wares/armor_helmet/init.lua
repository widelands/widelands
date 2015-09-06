dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "empire_ware",
   name = "armor_helmet",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("empire_ware", "Helmet"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("empire_ware", "helmets"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
		empire = 30
	},
   preciousness = {
		empire = 2
	},

   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 10 },
      },
   }
}
