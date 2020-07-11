dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "brick",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Brick"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 7, 6 },
      },
   }
}
