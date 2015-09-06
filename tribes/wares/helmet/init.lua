dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "barbarians_ware",
   name = "helmet",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("barbarians_ware", "Helmet"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("barbarians_ware", "helmets"),
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
         hotspot = { 7, 8 },
      },
   }
}
