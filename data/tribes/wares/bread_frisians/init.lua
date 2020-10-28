push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "bread_frisians",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("frisians_ware", "Bread"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 6, 6 },
      },
   }
}

pop_textdomain()
