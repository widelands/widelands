push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_ware_type {
   name = "cassavaroot",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Cassavaroot"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         hotspot = { 6, 6 },
      }
   },
}

pop_textdomain()
