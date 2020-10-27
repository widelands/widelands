push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "armor_helmet",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("empire_ware", "Helmet"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 5, 10 },
      },
   }
}

pop_textdomain()
