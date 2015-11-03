dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "grape",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Grape"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      empire = 20
   },
   preciousness = {
      empire = 10
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 7, 15 },
      },
   }
}
