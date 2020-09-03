push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_ware_type {
   name = "blackroot_flour",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Blackroot Flour"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 4, 15 },
      },
   }
}

pop_textdomain()
