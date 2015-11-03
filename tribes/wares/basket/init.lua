dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "basket",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Basket"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
      empire = 1
   },
   preciousness = {
      empire = 0
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 8, 11 },
      },
   }
}
