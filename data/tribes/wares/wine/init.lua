push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "wine",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Wine"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 4, 14 },
      },
   }
}

pop_textdomain()
