dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "kitchen_tools",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Kitchen Tools"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "kitchen tools"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
		barbarians = 1,
		empire = 1
	},
   preciousness = {
		barbarians = 0,
		empire = 0
	},

   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 8, 8 },
      },
   }
}
