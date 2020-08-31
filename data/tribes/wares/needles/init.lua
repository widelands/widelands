push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_ware_type {
   name = "needles",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Needles"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 4, 3 },
      },
   }
}

pop_textdomain()
