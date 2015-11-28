dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "cloth",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Cloth"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      barbarians = 10,
      empire = 15
   },
   preciousness = {
      barbarians = 0,
      empire = 1
   },

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 6, 9 },
      },
   }
}
