dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "hunting_bow",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Hunting Bow"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      atlanteans = 1
   },
   preciousness = {
      atlanteans = 0
   },

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 9, 16 },
      },
   }
}
