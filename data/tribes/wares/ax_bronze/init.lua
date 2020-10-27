push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "ax_bronze",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Bronze Ax"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 7, 7 },
      },
   }
}

pop_textdomain()
