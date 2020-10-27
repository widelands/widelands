push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "beer_strong",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Strong Beer"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 5, 8 },
      },
   }
}

pop_textdomain()
