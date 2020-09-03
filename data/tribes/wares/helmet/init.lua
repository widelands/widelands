push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_ware_type {
   name = "helmet",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Helmet"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 7, 8 },
      },
   }
}

pop_textdomain()
