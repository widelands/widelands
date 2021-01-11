push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "tunic",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Tunic"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 6, 9 },
      },
   }
}

pop_textdomain()
