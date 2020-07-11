dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "barley",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Barley"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 3, 6 },
      },
   }
}
