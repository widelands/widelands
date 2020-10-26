push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "scrap_iron",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Scrap Iron"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 7, 9 },
      },
   }
}

pop_textdomain()
