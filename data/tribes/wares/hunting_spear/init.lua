push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_ware_type {
   name = "hunting_spear",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Hunting Spear"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 6, 5 },
      },
   }
}

pop_textdomain()
