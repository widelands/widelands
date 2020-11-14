push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "gold_ore",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Gold Ore"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 4, 4 },
      },
   }
}

pop_textdomain()
