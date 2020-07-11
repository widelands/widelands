dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "basket",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Basket"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 8, 11 },
      },
   }
}
