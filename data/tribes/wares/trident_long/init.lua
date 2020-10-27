push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "trident_long",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Long Trident"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 10, 17 },
      },
   }
}

pop_textdomain()
