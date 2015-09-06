dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "helmet_mask",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Mask"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "masks"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
		barbarians = 1
	},
   preciousness = {
		barbarians = 1
	},

   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 10 },
      },
   }
}
