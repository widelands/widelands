push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "spidercloth",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Spidercloth"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 6, 9 },
      },
   }
}

pop_textdomain()
