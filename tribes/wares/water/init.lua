dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "water",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Water"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      atlanteans = 25,
      barbarians = 25,
      empire = 25
   },
   preciousness = {
      atlanteans = 7,
      barbarians = 8,
      empire = 7
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 4, 8 },
      },
   }
}
