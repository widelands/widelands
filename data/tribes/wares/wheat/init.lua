push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_ware_type {
   name = "wheat",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Wheat"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { -1, 6 },
      },
   }
}

pop_textdomain()
