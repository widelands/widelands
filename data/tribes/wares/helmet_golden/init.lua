push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "helmet_golden",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("frisians_ware", "Golden Helmet"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = {8, 6},
      },
   }
}

pop_textdomain()
