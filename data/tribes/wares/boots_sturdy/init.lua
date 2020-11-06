push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "boots_sturdy",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Sturdy Boots"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 9, 13 },
      },
   }
}

pop_textdomain()
