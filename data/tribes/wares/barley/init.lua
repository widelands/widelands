push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
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

pop_textdomain()
