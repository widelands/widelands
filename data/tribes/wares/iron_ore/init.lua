push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_ware_type {
   name = "iron_ore",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Iron Ore"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 4, 4 },
      },
   }
}

pop_textdomain()
