push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "smoked_fish",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Smoked Fish"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 9, 16 },
      },
   }
}

pop_textdomain()
