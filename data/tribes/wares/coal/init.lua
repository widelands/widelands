push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "coal",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Coal"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 4, 6 },
      },
   }
}

pop_textdomain()
