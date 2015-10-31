dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "granite",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Granite"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      atlanteans = 20,
      barbarians = 20,
      empire = 30
   },
   preciousness = {
      atlanteans = 5,
      barbarians = 5,
      empire = 0
   },

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 7, 8 },
      },
   }
}
