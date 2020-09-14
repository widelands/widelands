push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_ware_type {
   name = "liana",
   -- TRANSLATORS: This is a ware name used in lists of wares. Used to make ropes.
   descname = pgettext("ware", "Liana"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 3, 12 },
      },
   }
}

pop_textdomain()
