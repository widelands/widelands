dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "ax_battle",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Battle Ax"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "battle axes"),
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
         hotspot = { 7, 7 },
      },
   }
}
