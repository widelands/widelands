dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "thatch_reed",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Thatch Reed"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "thatch reed"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
		barbarians = 10
	},
   preciousness = {
		barbarians = 5
	},

   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 3, 12 },
      },
   }
}
