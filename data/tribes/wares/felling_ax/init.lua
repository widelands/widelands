push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_ware_type {
   name = "felling_ax",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Felling Ax"),
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 6, 7 },
      },
   }
}

pop_textdomain()
