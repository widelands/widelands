push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "rye",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Rye"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { -1, 6 },
      },
   }
}

pop_textdomain()
