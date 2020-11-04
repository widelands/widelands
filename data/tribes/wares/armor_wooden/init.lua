push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "armor_wooden",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Light Wooden Armor"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 3, 11 },
      },
   }
}

pop_textdomain()
