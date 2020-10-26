push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "diamond",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Diamond"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 4, 7 },
      },
   }
}

pop_textdomain()
