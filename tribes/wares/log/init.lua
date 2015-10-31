dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "log",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Log"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      atlanteans = 40,
      barbarians = 40,
      empire = 40
   },
   preciousness = {
      atlanteans = 14,
      barbarians = 14,
      empire = 14
   },

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 6, 7 },
      },
   }
}
