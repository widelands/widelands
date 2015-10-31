dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "iron",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Iron"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      atlanteans = 20,
      barbarians = 20,
      empire = 20
   },
   preciousness = {
      atlanteans = 4,
      barbarians = 4,
      empire = 4
   },

   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 7, 9 },
      },
   }
}
