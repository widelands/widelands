push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_ware_type {
   name = "marble",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Marble"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 10, 18 },
      },
   }
}

pop_textdomain()
