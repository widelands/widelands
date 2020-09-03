push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_ware_type {
   name = "snack",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Snack"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 5, 5 },
      },
   }
}

pop_textdomain()
