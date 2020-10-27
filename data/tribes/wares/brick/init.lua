push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "brick",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Brick"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 7, 6 },
      },
   }
}

pop_textdomain()
