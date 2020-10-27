push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "mead",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Mead"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 5, 9 },
      },
   }
}

pop_textdomain()
