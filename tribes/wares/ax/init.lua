dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "ax",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Ax"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "axes"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
		barbarians = 30
	},
   preciousness = {
		barbarians = 3
	},

   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 7 },
      },
   }
}
