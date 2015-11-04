dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "ax_warriors",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Warrior’s Ax"),
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
         template = "idle",
         directory = dirname,
         hotspot = { 7, 7 },
      },
   }
}
